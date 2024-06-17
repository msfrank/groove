
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/bar_annotation_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::BarAnnotationDatum::BarAnnotationDatum()
    : key(), value(), fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{
}

groove_shapes::BarAnnotationDatum::BarAnnotationDatum(
    const groove_data::Category &key,
    const std::string &value,
    groove_data::DatumFidelity fidelity)
    : key(key), value(value), fidelity(fidelity)
{
}

groove_shapes::BarAnnotationShape::BarAnnotationShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::BarAnnotationShape::configure(const SourceDescriptor &source)
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
        auto getModelResult = model->getIndexedColumn<groove_model::CategoryString>(columnId);
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
groove_shapes::BarAnnotationShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::BarAnnotationShape::getShapeType() const
{
    return DataShapeType::SHAPE_BAR_ANNOTATION;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarAnnotationShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarAnnotationShape::itemsEnd() const
{
    return m_items.cend();
}

tempo_utils::Result<groove_shapes::BarAnnotationDatum>
groove_shapes::BarAnnotationShape::getBarAnnotationValue(
    const std::string &itemId,
    const groove_data::Category &key)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);

    auto result = column->getValue(key);
    if (result.isStatus())
        return result.getStatus();
    auto datum = result.getResult();
    if (datum.fidelity == groove_data::DatumFidelity::FIDELITY_UNKNOWN)
        return BarAnnotationDatum();
    return BarAnnotationDatum(datum.key, datum.value, datum.fidelity);
}

tempo_utils::Result<groove_shapes::BarAnnotationDatumIterator>
groove_shapes::BarAnnotationShape::getBarAnnotationValues(
    const std::string &itemId,
    const groove_data::CategoryRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    return BarAnnotationDatumIterator(result.getResult());
}

std::vector<groove_shapes::BarAnnotationDatum>
groove_shapes::BarAnnotationShape::describeItemsAt(
    const groove_data::Category &fromKeyInclusive,
    const groove_data::Category &toKeyInclusive,
    const std::vector<std::string> &itemIds)
{
    groove_data::CategoryRange range;
    range.start = Option<groove_data::Category>(fromKeyInclusive);
    range.start_exclusive = false;
    range.end = Option<groove_data::Category>(toKeyInclusive);
    range.end_exclusive = false;

    std::vector<std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryString>>> columns;
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

    std::vector<BarAnnotationDatum> items;
    for (auto column : columns) {
        auto result = column->getValues(range);
        if (result.isStatus())
            continue;
        auto values = result.getResult();
        groove_data::CategoryStringDatum datum;
        while (values.getNext(datum)) {
            items.push_back(BarAnnotationDatum(datum.key, datum.value, datum.fidelity));
        }
    }

    return items;
}

groove_shapes::BarAnnotationDatumIterator::BarAnnotationDatumIterator()
{
}

groove_shapes::BarAnnotationDatumIterator::BarAnnotationDatumIterator(
    groove_model::CategoryStringColumnIterator input)
    : m_input(input)
{
}

bool
groove_shapes::BarAnnotationDatumIterator::getNext(BarAnnotationDatum &datum)
{
    groove_data::CategoryStringDatum next;
    if (!m_input.getNext(next))
        return false;
    datum = BarAnnotationDatum(next.key, next.value, next.fidelity);
    return true;
}
