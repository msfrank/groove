
#include <groove_shapes/base_shape.h>

groove_shapes::BaseShape::BaseShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : m_database(database),
      m_shapeId(shapeId)
{
    TU_ASSERT (m_database != nullptr);
    TU_ASSERT (!m_shapeId.empty());
}

std::shared_ptr<groove_model::GrooveDatabase>
groove_shapes::BaseShape::getDatabase() const
{
    return m_database;
}

std::shared_ptr<groove_model::AbstractDataset>
groove_shapes::BaseShape::getDataset(const tempo_utils::Url &datasetUrl) const
{
    return m_database->getDataset(datasetUrl);
}

std::shared_ptr<groove_model::GrooveModel>
groove_shapes::BaseShape::getModel(const tempo_utils::Url &datasetUrl, const std::string &modelId) const
{
    auto dataset = m_database->getDataset(datasetUrl);
    if (dataset == nullptr)
        return {};
    return dataset->getModel(modelId);
}
