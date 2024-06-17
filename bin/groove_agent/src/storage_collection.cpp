
#include <groove_agent/storage_collection.h>

StorageCollection::StorageCollection(
    const tempo_utils::Url &collectionUrl,
    groove_model::GrooveDatabase *db,
    int maxWatchers)
    : m_collectionUrl(collectionUrl),
      m_db(db),
      m_maxWatchers(maxWatchers),
      m_lastKey(0)
{
    TU_ASSERT (m_collectionUrl.isValid());
    TU_ASSERT (m_db != nullptr);
    m_lock = new absl::Mutex();
}

StorageCollection::~StorageCollection()
{
    delete m_lock;
}

tempo_utils::Url
StorageCollection::getCollectionUrl() const
{
    absl::MutexLock locker(m_lock);
    return m_collectionUrl;
}

bool
StorageCollection::hasDataset(const tempo_utils::Url &datasetUrl) const
{
    absl::MutexLock locker(m_lock);
    return m_datasets.contains(datasetUrl);
}

std::shared_ptr<groove_model::AbstractDataset>
StorageCollection::getDataset(const tempo_utils::Url &datasetUrl) const
{
    absl::MutexLock locker(m_lock);
    if (m_datasets.contains(datasetUrl))
        return m_datasets.at(datasetUrl);
    return {};
}

tempo_utils::Status
StorageCollection::declareDataset(const tempo_utils::Url &datasetUrl, groove_model::GrooveSchema schema)
{
    absl::MutexLock locker(m_lock);
    if (m_datasets.contains(datasetUrl))
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "dataset already exists");
    auto status = m_db->declareDataset(datasetUrl, schema);
    if (status.notOk())
        return status;
    auto dataset = m_db->getDataset(datasetUrl);
    m_datasets[datasetUrl] = dataset;
    return groove_storage::StorageStatus::ok();
}

tempo_utils::Status
StorageCollection::loadDatasetFile(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<groove_io::DatasetFile> datasetFile)
{
    auto schema = datasetFile->getSchema();

    std::shared_ptr<groove_model::AbstractDataset> dataset;
    {
        absl::MutexLock locker(m_lock);
        if (m_datasets.contains(datasetUrl))
            return groove_storage::StorageStatus::forCondition(
                groove_storage::StorageCondition::kStorageInvariant, "dataset already exists");
        auto status = m_db->declareDataset(datasetUrl, schema);
        if (status.notOk())
            return status;
        dataset = m_db->getDataset(datasetUrl);
        m_datasets[datasetUrl] = dataset;
    }

// TODO:
//    for (int i = 0; i < schema.numModels(); i++) {
//        const auto *modelDescriptor = schema.getModel(i);
//        auto modelSrc = datasetFile->getModel(modelDescriptor->id()->str());
//        auto modelDst = dataset->getModel(modelDescriptor->id()->str());
//    }

    return groove_storage::StorageStatus::ok();
}

tempo_utils::Status
StorageCollection::linkDatasetFile(const tempo_utils::Url &datasetUrl, const std::filesystem::path &path)
{
    auto readDatasetResult = groove_io::DatasetFile::create(path);
    if (readDatasetResult.isStatus())
        return readDatasetResult.getStatus();
    auto dataset = readDatasetResult.getResult();
    absl::MutexLock locker(m_lock);
    if (m_datasets.contains(datasetUrl))
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "dataset already exists");
    m_datasets[datasetUrl] = dataset;
    return groove_storage::StorageStatus::ok();
}

tempo_utils::Result<bool>
StorageCollection::removeDataset(const tempo_utils::Url &datasetUrl)
{
    absl::MutexLock locker(m_lock);
    if (!m_datasets.contains(datasetUrl))
        return false;
    m_datasets.erase(datasetUrl);
    return true;
}

tempo_utils::Result<tu_uint64>
StorageCollection::attachWatcher(AbstractCollectionWatcher *watcher)
{
    absl::MutexLock locker(m_lock);
    if (m_maxWatchers <= m_watchers.size())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "exceeded max watchers");
    auto watchKey = ++m_lastKey;
    m_watchers[watchKey] = watcher;
    return watchKey;
}

tempo_utils::Status
StorageCollection::detachWatcher(tu_uint64 watchKey)
{
    absl::MutexLock locker(m_lock);
    if (!m_watchers.contains(watchKey))
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "invalid watch key");
    m_watchers.erase(watchKey);
    return groove_storage::StorageStatus::ok();
}

tempo_utils::Status
StorageCollection::drop()
{
    return groove_storage::StorageStatus::forCondition(
        groove_storage::StorageCondition::kStorageInvariant, "drop is not implemented");
}
