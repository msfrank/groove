
#include <groove_iterator/map_iterator_template.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/summary_group_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::SummaryGroupDatum::SummaryGroupDatum()
    : range(), value{}, fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{
}

groove_shapes::SummaryGroupDatum::SummaryGroupDatum(
    const groove_data::DoubleRange &range,
    double value,
    groove_data::DatumFidelity fidelity)
    : range(range), value(value), fidelity(fidelity)
{
}

groove_shapes::SummaryGroupShape::SummaryGroupShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::SummaryGroupShape::configure(const SourceDescriptor &source)
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
groove_shapes::SummaryGroupShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::SummaryGroupShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_SUMMARY_GROUP;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SummaryGroupShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SummaryGroupShape::itemsEnd() const
{
    return m_items.cend();
}

groove_data::DataValueType
groove_shapes::SummaryGroupShape::getSummaryGroupColumnValueType(const std::string &itemId) const
{
    // FIXME: eventually we should support different column value types
    return groove_data::DataValueType::VALUE_TYPE_DOUBLE;
}

static double map_DoubleDoubleDatum_to_double(const groove_data::DoubleDoubleDatum &datum)
{
    return datum.value;
}

tempo_utils::Result<std::shared_ptr<Iterator<double>>>
groove_shapes::SummaryGroupShape::getItemValues(
    const std::string &itemId,
    const groove_data::DoubleRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    auto input = result.getResult();

    auto values = std::make_shared<
        groove_iterator::MapIterator<
            groove_model::DoubleDoubleColumnIterator,
            groove_data::DoubleDoubleDatum,
            double>>(input, map_DoubleDoubleDatum_to_double);

    return std::static_pointer_cast<Iterator<double>>(values);
}
