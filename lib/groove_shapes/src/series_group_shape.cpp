
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/series_group_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::SeriesGroupDatum::SeriesGroupDatum()
    : key(), value(), fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{

}

groove_shapes::SeriesGroupDatum::SeriesGroupDatum(
    double key,
    double value,
    groove_data::DatumFidelity fidelity)
    : key(key), value(value), fidelity(fidelity)
{
}

groove_shapes::SeriesGroupShape::SeriesGroupShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::SeriesGroupShape::configure(const SourceDescriptor &source)
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
        auto getModelResult = model->getIndexedColumn<groove_model::DoubleDouble>(columnId);
        if (getModelResult.isStatus())
            return getModelResult.getStatus();
        auto seriesId = absl::StrCat(
            source.getDatasetUrl().toString(),
            " ", source.getModelId(),
            " ", columnId);
        m_columns[seriesId] = getModelResult.getResult();
        m_items.insert(seriesId);
    }

    return ShapesStatus::ok();
}

bool
groove_shapes::SeriesGroupShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::SeriesGroupShape::getShapeType() const
{
    return DataShapeType::SHAPE_BAR_ANNOTATION;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesGroupShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesGroupShape::itemsEnd() const
{
    return m_items.cend();
}

tempo_utils::Result<groove_shapes::SeriesGroupDatum>
groove_shapes::SeriesGroupShape::getSeriesGroupValue(
    const std::string &seriesId,
    double key)
{
    if (!m_columns.contains(seriesId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(seriesId);

    auto result = column->getValue(key);
    if (result.isStatus())
        return result.getStatus();
    auto datum = result.getResult();
    if (datum.fidelity == groove_data::DatumFidelity::FIDELITY_UNKNOWN)
        return SeriesGroupDatum();
    return SeriesGroupDatum(datum.key, datum.value, datum.fidelity);
}

tempo_utils::Result<groove_shapes::SeriesGroupDatumIterator>
groove_shapes::SeriesGroupShape::getSeriesGroupValues(
    const std::string &seriesId,
    const groove_data::DoubleRange &range)
{
    if (!m_columns.contains(seriesId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(seriesId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    return SeriesGroupDatumIterator(result.getResult());
}

std::vector<groove_shapes::SeriesGroupDatum>
groove_shapes::SeriesGroupShape::describeItemsAt(
    double fromKeyInclusive,
    double toKeyInclusive,
    const std::vector<std::string> &seriesIds)
{
    groove_data::DoubleRange range;
    range.start = Option<double>(fromKeyInclusive);
    range.start_exclusive = false;
    range.end = Option<double>(toKeyInclusive);
    range.end_exclusive = false;

    std::vector<std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleDouble>>> columns;
    if (seriesIds.empty()) {
        for (auto iterator = m_columns.begin(); iterator != m_columns.end(); iterator++) {
            columns.push_back(iterator->second);
        }
    } else {
        for (const auto &seriesId : seriesIds) {
            if (m_columns.contains(seriesId)) {
                columns.push_back(m_columns.at(seriesId));
            }
        }
    }

    std::vector<SeriesGroupDatum> items;
    for (auto column : columns) {
        auto result = column->getValues(range);
        if (result.isStatus())
            continue;
        auto values = result.getResult();
        groove_data::DoubleDoubleDatum datum;
        while (values.getNext(datum)) {
            items.push_back(SeriesGroupDatum(datum.key, datum.value, datum.fidelity));
        }
    }

    return items;
}

groove_shapes::SeriesGroupDatumIterator::SeriesGroupDatumIterator()
{
}

groove_shapes::SeriesGroupDatumIterator::SeriesGroupDatumIterator(
    groove_model::DoubleDoubleColumnIterator input)
    : m_input(input)
{
}

bool
groove_shapes::SeriesGroupDatumIterator::getNext(SeriesGroupDatum &datum)
{
    groove_data::DoubleDoubleDatum next;
    if (!m_input.getNext(next))
        return false;
    datum = SeriesGroupDatum(next.key, next.value, next.fidelity);
    return true;
}
