
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/pie_group_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::PieGroupDatum::PieGroupDatum()
    : id(), values(), total(0.0)
{
}

groove_shapes::PieGroupDatum::PieGroupDatum(
    const std::string &id,
    const absl::flat_hash_map<groove_data::Category, std::pair<double,groove_data::DatumFidelity>> &values,
    double total)
    : id(id), values(values), total(total)
{
}

groove_shapes::PieGroupShape::PieGroupShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::PieGroupShape::configure(const SourceDescriptor &source)
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
groove_shapes::PieGroupShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::PieGroupShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_PIE_GROUP;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::PieGroupShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::PieGroupShape::itemsEnd() const
{
    return m_items.cend();
}

tempo_utils::Result<groove_shapes::PieGroupDatum>
groove_shapes::PieGroupShape::getPieGroupValue(
    const std::string &itemId,
    const groove_data::CategoryRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);

    auto result = column->getValues(range);
    if(result.isStatus())
        return result.getStatus();
    auto it = result.getResult();

    absl::flat_hash_map<groove_data::Category, std::pair<double,groove_data::DatumFidelity>> values;
    double total = 0.0;

    groove_data::CategoryDoubleDatum datum;
    while (it.getNext(datum)) {
        values[datum.key] = std::pair<double,groove_data::DatumFidelity>{datum.value, datum.fidelity};
        total += datum.value;
    }

    return PieGroupDatum(itemId, std::move(values), total);
}

tempo_utils::Result<groove_shapes::PieGroupDatum>
groove_shapes::PieGroupShape::getPieGroupValue(const std::string &itemId)
{
    groove_data::CategoryRange range;
    range.start = Option<groove_data::Category>();
    range.start_exclusive = false;
    range.end = Option<groove_data::Category>();
    range.end_exclusive = false;
    return getPieGroupValue(itemId, range);
}