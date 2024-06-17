
#include <groove_model/base_column.h>

groove_model::BaseColumn::BaseColumn(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_columnId(columnId)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_modelId != nullptr && !m_modelId->empty());
    TU_ASSERT (m_columnId != nullptr && !m_columnId->empty());
}

tempo_utils::Url
groove_model::BaseColumn::getDatasetUrl() const
{
    return m_datasetUrl;
}

std::shared_ptr<const std::string>
groove_model::BaseColumn::getModelId() const
{
    return m_modelId;
}

std::shared_ptr<const std::string>
groove_model::BaseColumn::getColumnId() const
{
    return m_columnId;
}