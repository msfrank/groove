
#include <groove_storage/storage_types.h>

groove_storage::BaseSyncRequest::BaseSyncRequest(
    SyncRequestType type,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId)
    : m_type(type),
      m_datasetUrl(datasetUrl),
      m_modelId(modelId)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
}

groove_storage::BaseSyncRequest::SyncRequestType
groove_storage::BaseSyncRequest::getType() const
{
    return m_type;
}

tempo_utils::Url
groove_storage::BaseSyncRequest::getDatasetUrl() const
{
    return m_datasetUrl;
}

std::string
groove_storage::BaseSyncRequest::getModelId() const
{
    return m_modelId;
}

groove_storage::CategorySyncRequest::CategorySyncRequest(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::CategoryRange &range)
    : BaseSyncRequest(SyncRequestType::CATEGORY_REQUEST, datasetUrl, modelId),
      m_range(range)
{
}

groove_data::CategoryRange
groove_storage::CategorySyncRequest::getRange() const
{
    return m_range;
}

groove_storage::DoubleSyncRequest::DoubleSyncRequest(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::DoubleRange &range)
    : BaseSyncRequest(SyncRequestType::DOUBLE_REQUEST, datasetUrl, modelId),
      m_range(range)
{
}

groove_data::DoubleRange
groove_storage::DoubleSyncRequest::getRange() const
{
    return m_range;
}

groove_storage::Int64SyncRequest::Int64SyncRequest(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::Int64Range &range)
    : BaseSyncRequest(SyncRequestType::INT64_REQUEST, datasetUrl, modelId),
      m_range(range)
{
}

groove_data::Int64Range
groove_storage::Int64SyncRequest::getRange() const
{
    return m_range;
}