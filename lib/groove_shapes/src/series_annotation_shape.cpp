
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/series_annotation_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::SeriesAnnotationDatum::SeriesAnnotationDatum()
    : key{}, value(), fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{
}

groove_shapes::SeriesAnnotationDatum::SeriesAnnotationDatum(
    double key,
    const std::string &value,
    groove_data::DatumFidelity fidelity)
    : key(key), value(value), fidelity(fidelity)
{
}

groove_shapes::SeriesAnnotationShape::SeriesAnnotationShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &itemId)
    : BaseShape(database, itemId)
{
}

tempo_utils::Status
groove_shapes::SeriesAnnotationShape::configure(const SourceDescriptor &source)
{
    auto model = getModel(source.getDatasetUrl(), source.getModelId());
    std::vector<std::string> columnIds;
    if (!source.getColumnId().empty()) {
        columnIds.push_back(source.getColumnId());
    } else {
        for (auto iterator = model->columnsBegin(); iterator != model->columnsEnd(); iterator++) {
            columnIds.push_back(iterator->first);
        }
    }

    for (const auto &columnId : columnIds) {
        auto getModelResult = model->getIndexedColumn<groove_model::DoubleString>(columnId);
        if (getModelResult.isStatus())
            return getModelResult.getStatus();
        auto itemId = absl::StrCat(
            source.getDatasetUrl().toString(),
            " ", source.getModelId(),
            " ", columnId);
        m_columns[itemId] = getModelResult.getResult();
        m_items.insert(itemId);
    }

    return ShapesStatus::ok();
}

bool
groove_shapes::SeriesAnnotationShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::SeriesAnnotationShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_SERIES_ANNOTATION;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesAnnotationShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesAnnotationShape::itemsEnd() const
{
    return m_items.cend();
}

groove_units::UnitDimension
groove_shapes::SeriesAnnotationShape::getSeriesAnnotationUnits(const std::string &itemId)
{
//    auto source = getSource(seriesId);
//    auto descriptor = source.getStreamDescriptor();
//    auto column = source.getStreamColumn();
//    return descriptor->getIndexUnitDimension();
    return groove_units::UnitDimension();
}

tempo_utils::Result<groove_shapes::SeriesAnnotationDatum>
groove_shapes::SeriesAnnotationShape::getSeriesAnnotationValue(const std::string &itemId, double key)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);

    auto result = column->getValue(key);
    if (result.isStatus())
        return result.getStatus();
    auto datum = result.getResult();
    if (datum.fidelity == groove_data::DatumFidelity::FIDELITY_UNKNOWN)
        return SeriesAnnotationDatum();
    return SeriesAnnotationDatum(datum.key, datum.value, datum.fidelity);
}

tempo_utils::Result<groove_shapes::SeriesAnnotationDatumIterator>
groove_shapes::SeriesAnnotationShape::getSeriesAnnotationValues(
    const std::string &itemId,
    const groove_data::DoubleRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    return SeriesAnnotationDatumIterator(result.getResult());
}

std::vector<groove_shapes::SeriesAnnotationDatum>
groove_shapes::SeriesAnnotationShape::describeItemsAt(
    double fromKeyInclusive,
    double toKeyInclusive,
    const std::vector<std::string> &itemIds)
{
    groove_data::DoubleRange range;
    range.start = Option<double>(fromKeyInclusive);
    range.start_exclusive = false;
    range.end = Option<double>(toKeyInclusive);
    range.end_exclusive = false;

    std::vector<std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleString>>> columns;
    if (itemIds.empty()) {
        for (auto iterator = m_columns.begin(); iterator != m_columns.end(); iterator++) {
            columns.push_back(iterator->second);
        }
    } else {
        for (const auto &itemId : itemIds) {
            if (m_columns.contains(itemId)) {
                columns.push_back(m_columns.at(itemId));
            }
        }
    }

    std::vector<SeriesAnnotationDatum> items;
    for (auto column : columns) {
        auto result = column->getValues(range);
        if (result.isStatus())
            continue;
        auto values = result.getResult();
        groove_data::DoubleStringDatum datum;
        while (values.getNext(datum)) {
            items.push_back(SeriesAnnotationDatum(datum.key, datum.value, datum.fidelity));
        }
    }

    return items;
}

groove_shapes::SeriesAnnotationDatumIterator::SeriesAnnotationDatumIterator()
{
}

groove_shapes::SeriesAnnotationDatumIterator::SeriesAnnotationDatumIterator(
    groove_model::DoubleStringColumnIterator input)
    : m_input(input)
{
}

bool
groove_shapes::SeriesAnnotationDatumIterator::getNext(SeriesAnnotationDatum &datum)
{
    groove_data::DoubleStringDatum next;
    if (!m_input.getNext(next))
        return false;
    datum = SeriesAnnotationDatum(next.key, next.value, next.fidelity);
    return true;
}