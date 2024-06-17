
#include <groove_shapes/shape_types.h>

groove_shapes::SourceDescriptor::SourceDescriptor()
{
}

groove_shapes::SourceDescriptor::SourceDescriptor(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::string &columnId)
{
    TU_ASSERT (datasetUrl.isValid());
    TU_ASSERT (!modelId.empty());
    TU_ASSERT (!columnId.empty());

    SourcePriv priv;
    priv.datasetUrl = datasetUrl;
    priv.modelId = modelId;
    priv.columnId = columnId;
    m_priv = std::make_shared<SourcePriv>(std::move(priv));
}

groove_shapes::SourceDescriptor::SourceDescriptor(const tempo_utils::Url &datasetUrl, const std::string &modelId)
{
    TU_ASSERT (datasetUrl.isValid());
    TU_ASSERT (!modelId.empty());

    SourcePriv priv;
    priv.datasetUrl = datasetUrl;
    priv.modelId = modelId;
    m_priv = std::make_shared<SourcePriv>(std::move(priv));
}

groove_shapes::SourceDescriptor::SourceDescriptor(const SourceDescriptor &other)
    : m_priv(other.m_priv)
{
}

bool
groove_shapes::SourceDescriptor::isValid() const
{
    return m_priv != nullptr;
}

tempo_utils::Url
groove_shapes::SourceDescriptor::getDatasetUrl() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->datasetUrl;
}

std::string
groove_shapes::SourceDescriptor::getModelId() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->modelId;
}

std::string
groove_shapes::SourceDescriptor::getColumnId() const
{
    if (m_priv == nullptr)
        return {};
    return m_priv->columnId;
}

groove_shapes::ShapeDef::ShapeDef()
    : m_type(DataShapeType::SHAPE_UNKNOWN)
{
}

groove_shapes::ShapeDef::ShapeDef(DataShapeType type)
    : m_type(type)
{
}

groove_shapes::ShapeDef::ShapeDef(const ShapeDef &other)
    : m_type(other.m_type)
{
}

groove_shapes::DataShapeType
groove_shapes::ShapeDef::getType() const
{
    return m_type;
}

groove_shapes::BarAnnotation::BarAnnotation()
    : ShapeDef(DataShapeType::SHAPE_BAR_ANNOTATION)
{
}

groove_shapes::BarGroup::BarGroup()
    : ShapeDef(DataShapeType::SHAPE_BAR_GROUP)
{
}

groove_shapes::BarStack::BarStack()
    : ShapeDef(DataShapeType::SHAPE_BAR_STACK)
{
}

groove_shapes::EventTable::EventTable()
    : ShapeDef(DataShapeType::SHAPE_EVENT_TABLE)
{
}

groove_shapes::InfoTable::InfoTable()
    : ShapeDef(DataShapeType::SHAPE_INFO_TABLE)
{
}

groove_shapes::PieAnnotation::PieAnnotation()
    : ShapeDef(DataShapeType::SHAPE_PIE_ANNOTATION)
{
}

groove_shapes::PieGroup::PieGroup()
    : ShapeDef(DataShapeType::SHAPE_PIE_GROUP)
{
}

groove_shapes::SeriesAnnotation::SeriesAnnotation()
    : ShapeDef(DataShapeType::SHAPE_SERIES_ANNOTATION)
{
}

groove_shapes::SeriesGroup::SeriesGroup()
    : ShapeDef(DataShapeType::SHAPE_SERIES_GROUP)
{
}

groove_shapes::SeriesStack::SeriesStack()
    : ShapeDef(DataShapeType::SHAPE_SERIES_STACK)
{
}

groove_shapes::SummaryGroup::SummaryGroup()
    : ShapeDef(DataShapeType::SHAPE_SUMMARY_GROUP)
{
}