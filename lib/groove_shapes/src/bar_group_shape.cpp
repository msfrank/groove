
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/bar_group_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::BarGroupDatum::BarGroupDatum()
    : key(), value{}, fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{
}

groove_shapes::BarGroupDatum::BarGroupDatum(
    const groove_data::Category &key,
    double value,
    groove_data::DatumFidelity fidelity)
    : key(key), value(value), fidelity(fidelity)
{
}

groove_shapes::BarGroupShape::BarGroupShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::BarGroupShape::configure(const SourceDescriptor &source)
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
        auto getModelResult = model->getIndexedColumn<groove_model::CategoryDouble>(columnId);
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
groove_shapes::BarGroupShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::BarGroupShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_BAR_GROUP;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarGroupShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarGroupShape::itemsEnd() const
{
    return m_items.cend();
}

groove_units::UnitDimension
groove_shapes::BarGroupShape::getBarGroupUnits(const std::string &itemId)
{
//    auto source = getSource(categoryId);
//    auto descriptor = source.getStreamDescriptor();
//    auto column = source.getStreamColumn();
//    return column->getUnitDimension();
    return groove_units::UnitDimension();
}

tempo_utils::Result<groove_shapes::BarGroupDatum>
groove_shapes::BarGroupShape::getBarGroupValue(const std::string &itemId, const groove_data::Category &key)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);

    auto result = column->getValue(key);
    if (result.isStatus())
        return result.getStatus();
    auto datum = result.getResult();
    if (datum.fidelity == groove_data::DatumFidelity::FIDELITY_UNKNOWN)
        return BarGroupDatum();
    return BarGroupDatum(datum.key, datum.value, datum.fidelity);
}

tempo_utils::Result<groove_shapes::BarGroupDatumIterator>
groove_shapes::BarGroupShape::getBarGroupValues(
    const std::string &itemId,
    const groove_data::CategoryRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    return BarGroupDatumIterator(result.getResult());
}

std::vector<groove_shapes::BarGroupDatum>
groove_shapes::BarGroupShape::describeItemsAt(const groove_data::Category &key, double value, double hitradius)
{
    std::vector<BarGroupDatum> items;
    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto column = iterator->second;
        auto result = column->getValue(key);
        if (result.isStatus())
            continue;
        auto datum = result.getResult();
        items.push_back(BarGroupDatum(datum.key, datum.value, datum.fidelity));
    }

    return items;
}

groove_shapes::BarGroupDatumIterator::BarGroupDatumIterator()
{
}

groove_shapes::BarGroupDatumIterator::BarGroupDatumIterator(
    groove_model::CategoryDoubleColumnIterator input)
    : m_input(input)
{
}

bool
groove_shapes::BarGroupDatumIterator::getNext(BarGroupDatum &datum)
{
    groove_data::CategoryDoubleDatum next;
    if (!m_input.getNext(next))
        return false;
    datum = BarGroupDatum(next.key, next.value, next.fidelity);
    return true;
}
