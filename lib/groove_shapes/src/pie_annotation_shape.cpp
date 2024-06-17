
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/pie_annotation_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::PieAnnotationDatum::PieAnnotationDatum()
    : key(), value(), fidelity(groove_data::DatumFidelity::FIDELITY_UNKNOWN)
{
}

groove_shapes::PieAnnotationDatum::PieAnnotationDatum(
    const groove_data::Category &key,
    const std::string &value,
    groove_data::DatumFidelity fidelity)
    : key(key), value(value), fidelity(fidelity)
{
}

groove_shapes::PieAnnotationShape::PieAnnotationShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::PieAnnotationShape::configure(const SourceDescriptor &source)
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
groove_shapes::PieAnnotationShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::PieAnnotationShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_PIE_ANNOTATION;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::PieAnnotationShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::PieAnnotationShape::itemsEnd() const
{
    return m_items.cend();
}

tempo_utils::Result<groove_shapes::PieAnnotationDatum>
groove_shapes::PieAnnotationShape::getPieAnnotationValue(
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
        return PieAnnotationDatum();
    return PieAnnotationDatum(datum.key, datum.value, datum.fidelity);
}

tempo_utils::Result<groove_shapes::PieAnnotationDatumIterator>
groove_shapes::PieAnnotationShape::getPieAnnotationValues(
    const std::string &itemId,
    const groove_data::CategoryRange &range)
{
    if (!m_columns.contains(itemId))
        return ShapesStatus::forCondition(ShapesCondition::kMissingItem);
    auto column = m_columns.at(itemId);
    auto result = column->getValues(range);
    if (result.isStatus())
        return result.getStatus();
    return PieAnnotationDatumIterator(result.getResult());
}

groove_shapes::PieAnnotationDatumIterator::PieAnnotationDatumIterator()
{
}

groove_shapes::PieAnnotationDatumIterator::PieAnnotationDatumIterator(
    groove_model::CategoryStringColumnIterator input)
    : m_input(input)
{
}

bool
groove_shapes::PieAnnotationDatumIterator::getNext(PieAnnotationDatum &datum)
{
    groove_data::CategoryStringDatum next;
    if (!m_input.getNext(next))
        return false;
    datum = PieAnnotationDatum(next.key, next.value, next.fidelity);
    return true;
}
