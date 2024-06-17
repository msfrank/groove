
#include <groove_storage/dataset_synchronizer.h>

groove_storage::DatasetSynchronizer::DatasetSynchronizer(
    std::shared_ptr<groove_model::GrooveDatabase> db,
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<SyncingClient> client)
    : m_db(db),
      m_datasetUrl(datasetUrl),
      m_client(client)
{
    TU_ASSERT (m_db != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_client != nullptr);
}

bool
groove_storage::DatasetSynchronizer::isValid() const
{
    return true;
}

tempo_utils::Status
groove_storage::DatasetSynchronizer::synchronizeSchema(std::shared_ptr<SchemaSyncNotification> notifier)
{
    auto synchronizer = SynchronizeSchemaHandler::create(
        m_datasetUrl, shared_from_this(), m_client, notifier);
    if (!m_client->describeDataset(m_datasetUrl, synchronizer))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "failed to synchronize schema");
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::DatasetSynchronizer::synchronizeModel(
    const std::string &modelId,
    const groove_data::CategoryRange &range,
    std::shared_ptr<ModelSyncNotification> notifier)
{
    auto synchronizer = SynchronizeModelHandler::create(
        m_datasetUrl, modelId, shared_from_this(), m_client, notifier);
    if (!m_client->getShards(m_datasetUrl, modelId, range, synchronizer))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "failed to synchronize model");
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::DatasetSynchronizer::synchronizeModel(
    const std::string &modelId,
    const groove_data::DoubleRange &range,
    std::shared_ptr<ModelSyncNotification> notifier)
{
    auto synchronizer = SynchronizeModelHandler::create(
        m_datasetUrl, modelId, shared_from_this(), m_client, notifier);
    if (!m_client->getShards(m_datasetUrl, modelId, range, synchronizer))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "failed to synchronize model");
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::DatasetSynchronizer::synchronizeModel(
    const std::string &modelId,
    const groove_data::Int64Range &range,
    std::shared_ptr<ModelSyncNotification> notifier)
{
    auto synchronizer = SynchronizeModelHandler::create(
        m_datasetUrl, modelId, shared_from_this(), m_client, notifier);
    if (!m_client->getShards(m_datasetUrl, modelId, range, synchronizer))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "failed to synchronize model");
    return StorageStatus::ok();
}

void
groove_storage::DatasetSynchronizer::updateSchema(groove_model::GrooveSchema schema)
{
    absl::MutexLock locker(&m_lock);    // FIXME: do we need to synchronize write operations to db?
    m_schema = schema;
    if (!m_db->hasDataset(m_datasetUrl)) {
        auto status = m_db->declareDataset(m_datasetUrl, m_schema);
        if (status.isOk()) {
            TU_LOG_INFO << "declared dataset " << m_datasetUrl;
        } else {
            TU_LOG_ERROR << "failed to declare dataset " << m_datasetUrl << ": " << status;
        }
    }
}

void
groove_storage::DatasetSynchronizer::updateModel(
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame)
{
    absl::MutexLock locker(&m_lock);    // FIXME: do we need to synchronize write operations to db?
    auto status = m_db->updateModel(m_datasetUrl, modelId, frame);
    if (status.isOk()) {
        TU_LOG_INFO << "updated model " << modelId
            << " with " << frame->numVectors() << " vectors"
            << " and " << frame->getSize() << " rows";
    } else {
        TU_LOG_ERROR << "failed to update model " << modelId << ": " << status;
    }
}

tempo_utils::Result<std::shared_ptr<groove_storage::DatasetSynchronizer>>
groove_storage::DatasetSynchronizer::create(
    std::shared_ptr<groove_model::GrooveDatabase> db,
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<SyncingClient> client)
{
    auto synchronizer = std::shared_ptr<DatasetSynchronizer>(new DatasetSynchronizer(db, datasetUrl, client));
    return synchronizer;
}

void
groove_storage::SchemaSyncNotification::schemaSyncFinished(
    const tempo_utils::Url &datasetUrl,
    const groove_model::GrooveSchema &schema,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::ModelSyncNotification::modelSyncFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

groove_storage::SynchronizeSchemaHandler::SynchronizeSchemaHandler(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<groove_storage::DatasetSynchronizer> synchronizer,
    std::shared_ptr<groove_storage::SyncingClient> client,
    std::shared_ptr<groove_storage::SchemaSyncNotification> notifier)
    : m_datasetUrl(datasetUrl),
      m_synchronizer(synchronizer),
      m_client(client),
      m_notifier(notifier)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_client != nullptr);
    TU_ASSERT (m_notifier != nullptr);
}

void
groove_storage::SynchronizeSchemaHandler::describeDatasetFinished(
    const tempo_utils::Url &datasetUrl,
    const groove_model::GrooveSchema &schema,
    const tempo_utils::Status &status)
{
    if (status.isOk()) {
        m_synchronizer->updateSchema(schema);
        m_notifier->schemaSyncFinished(m_datasetUrl, schema, groove_storage::StorageStatus::ok());
    } else {
        m_notifier->schemaSyncFinished(m_datasetUrl, schema, status);
    }
}

std::shared_ptr<groove_storage::SynchronizeSchemaHandler>
groove_storage::SynchronizeSchemaHandler::create(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<groove_storage::DatasetSynchronizer> synchronizer,
    std::shared_ptr<groove_storage::SyncingClient> client,
    std::shared_ptr<groove_storage::SchemaSyncNotification> notifier)
{
    return std::shared_ptr<SynchronizeSchemaHandler>(
        new SynchronizeSchemaHandler(datasetUrl, synchronizer, client, notifier));
}

groove_storage::SynchronizeModelHandler::SynchronizeModelHandler(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_storage::DatasetSynchronizer> synchronizer,
    std::shared_ptr<groove_storage::SyncingClient> client,
    std::shared_ptr<groove_storage::ModelSyncNotification> notifier)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_synchronizer(synchronizer),
      m_client(client),
      m_notifier(notifier)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_client != nullptr);
    TU_ASSERT (m_notifier != nullptr);
}

void
groove_storage::SynchronizeModelHandler::getShardsFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::vector<std::string> &shards,
    const tempo_utils::Status &status)
{
    if (status.notOk()) {
        m_notifier->modelSyncFinished(m_datasetUrl, m_modelId, status);
        return;
    }

    int count = 0;
    for (const auto &shard : shards) {
        m_shards.push_front(shard);
        count++;
    }
    TU_LOG_INFO << "received " << count << " shards for model " << m_modelId;
    if (count == 0) {
        m_notifier->modelSyncFinished(m_datasetUrl, m_modelId, groove_storage::StorageStatus::ok());
        return;
    }

    auto shard = m_shards.front();
    m_shards.pop_front();
    if (!m_client->getData(datasetUrl, modelId, shard, shared_from_this())) {
        m_notifier->modelSyncFinished(m_datasetUrl, m_modelId,
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, "failed to get data"));
    }
}

void
groove_storage::SynchronizeModelHandler::categoryFrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::CategoryFrame> frame)
{
    m_synchronizer->updateModel(modelId, frame);
}

void
groove_storage::SynchronizeModelHandler::doubleFrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::DoubleFrame> frame)
{
    m_synchronizer->updateModel(modelId, frame);
}

void
groove_storage::SynchronizeModelHandler::int64FrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::Int64Frame> frame)
{
    m_synchronizer->updateModel(modelId, frame);
}

void
groove_storage::SynchronizeModelHandler::getDataFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const tempo_utils::Status &status)
{
    if (status.notOk()) {
        m_notifier->modelSyncFinished(m_datasetUrl, m_modelId, status);
        return;
    }
    if (m_shards.empty()) {
        m_notifier->modelSyncFinished(m_datasetUrl, m_modelId, groove_storage::StorageStatus::ok());
        return;
    }
    auto shard = m_shards.front();
    m_shards.pop_front();
    m_client->getData(datasetUrl, modelId, shard, shared_from_this());
}

std::shared_ptr<groove_storage::SynchronizeModelHandler>
groove_storage::SynchronizeModelHandler::create(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_storage::DatasetSynchronizer> synchronizer,
    std::shared_ptr<groove_storage::SyncingClient> client,
    std::shared_ptr<groove_storage::ModelSyncNotification> notifier)
{
    return std::shared_ptr<SynchronizeModelHandler>(
        new SynchronizeModelHandler(datasetUrl, modelId, synchronizer, client, notifier));
}
