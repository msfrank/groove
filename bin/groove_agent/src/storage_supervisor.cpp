
#include <boost/uuid/uuid_io.hpp>

#include <groove_agent/storage_supervisor.h>

StorageSupervisor::StorageSupervisor(groove_model::GrooveDatabase *db)
    : m_db(db)
{
    TU_ASSERT (m_db != nullptr);
    m_lock = new absl::Mutex();
}

StorageSupervisor::~StorageSupervisor()
{
    delete m_lock;
}

bool
StorageSupervisor::hasCollection(std::string_view name) const
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);
    auto url = tempo_utils::Url::fromString(absl::StrCat("/", name));
    return m_collections.contains(url);
}

bool
StorageSupervisor::hasCollection(const tempo_utils::Url &url) const
{
    TU_ASSERT (url.isValid());

    std::filesystem::path path(url.getPath(), std::filesystem::path::generic_format);
    if (path.begin() == path.end())
        return false;
    auto name = std::string(*path.begin());
    return hasCollection(name);
}

std::shared_ptr<StorageCollection>
StorageSupervisor::getCollection(std::string_view name) const
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);
    auto url = tempo_utils::Url::fromString(absl::StrCat("/", name));
    if (m_collections.contains(url))
        return m_collections.at(url);
    return {};
}

std::shared_ptr<StorageCollection>
StorageSupervisor::getCollection(const tempo_utils::Url &url) const
{
    TU_ASSERT (url.isValid());

    std::filesystem::path path(url.getPath(), std::filesystem::path::generic_format);
    if (path.begin() == path.end())
        return {};
    auto name = std::string(*path.begin());
    return getCollection(name);
}

tempo_utils::Result<std::pair<std::shared_ptr<StorageCollection>,bool>>
StorageSupervisor::getOrCreateCollection(std::string_view name)
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);

    auto url = tempo_utils::Url::fromString(absl::StrCat("/", name));
    if (m_collections.contains(url))
        return std::pair<std::shared_ptr<StorageCollection>,bool>{m_collections.at(url),false};

    auto collection = std::make_shared<StorageCollection>(url, m_db, kMaxWatchersDefault);
    m_collections[url] = collection;
    return std::pair<std::shared_ptr<StorageCollection>,bool>{collection,true};
}

tempo_utils::Result<std::shared_ptr<StorageCollection>>
StorageSupervisor::createCollection(std::string_view name)
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);

    auto url = tempo_utils::Url::fromString(absl::StrCat("/", name));
    if (m_collections.contains(url))
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "collection already exists");

    auto collection = std::make_shared<StorageCollection>(url, m_db, kMaxWatchersDefault);
    m_collections[url] = collection;
    return collection;
}

tempo_utils::Result<std::shared_ptr<StorageCollection>>
StorageSupervisor::createEphemeralCollection(std::string_view name)
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);

    auto uuid = boost::uuids::to_string(m_uuidgen());
    auto url = tempo_utils::Url::fromString(absl::StrCat("/", uuid, "-", name));
    auto collection = std::make_shared<StorageCollection>(url, m_db, kMaxWatchersDefault);
    m_collections[url] = collection;
    return collection;
}

tempo_utils::Result<bool>
StorageSupervisor::deleteCollection(std::string_view name)
{
    TU_ASSERT (!name.empty());

    absl::MutexLock locker(m_lock);

    auto url = tempo_utils::Url::fromString(absl::StrCat("/", name));
    if (!m_collections.contains(url))
        return false;

    auto collection = m_collections.at(url);
    auto status = collection->drop();
    if (status.notOk())
        return status;
    m_collections.erase(url);
    return true;
}

bool
StorageSupervisor::hasDataset(const tempo_utils::Url &datasetUrl) const
{
    TU_ASSERT (datasetUrl.isValid());

    std::filesystem::path path(datasetUrl.getPath(), std::filesystem::path::generic_format);
    auto curr = path.begin();
    if (curr == path.end() || ++curr == path.end())
        return {};
    std::string collectionName(*curr);
    auto collectionUrl = tempo_utils::Url::fromString(absl::StrCat("/", collectionName));

    absl::MutexLock locker(m_lock);
    if (!m_collections.contains(collectionUrl))
        return false;
    auto collection = m_collections.at(collectionUrl);
    return collection->hasDataset(datasetUrl);
}

std::shared_ptr<groove_model::AbstractDataset>
StorageSupervisor::getDataset(const tempo_utils::Url &datasetUrl) const
{
    TU_ASSERT (datasetUrl.isValid());

    std::filesystem::path path(datasetUrl.getPath(), std::filesystem::path::generic_format);
    auto curr = path.begin();
    if (curr == path.end() || ++curr == path.end())
        return {};
    std::string collectionName(*curr);
    auto collectionUrl = tempo_utils::Url::fromString(absl::StrCat("/", collectionName));

    absl::MutexLock locker(m_lock);
    if (!m_collections.contains(collectionUrl))
        return {};
    auto collection = m_collections.at(collectionUrl);
    return collection->getDataset(datasetUrl);
}

tempo_utils::Result<std::shared_ptr<groove_model::AbstractDataset>>
StorageSupervisor::createDataset(const tempo_utils::Url &datasetUrl, const groove_model::GrooveSchema &schema)
{
    TU_ASSERT (datasetUrl.isValid());
    TU_ASSERT (schema.isValid());

    std::filesystem::path path(datasetUrl.getPath(), std::filesystem::path::generic_format);
    auto curr = path.begin();
    if (curr == path.end() || ++curr == path.end())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "invalid dataset url");
    std::string collectionName(*curr);
    auto collection = getCollection(collectionName);
    if (collection == nullptr)
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "dataset does not exist");

    auto status = collection->declareDataset(datasetUrl, schema);
    if (status.notOk())
        return status;
    auto dataset = collection->getDataset(datasetUrl);
    if (dataset == nullptr)
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "declared dataset does not exist");
    return dataset;
}

tempo_utils::Result<bool>
StorageSupervisor::deleteDataset(const tempo_utils::Url &datasetUrl)
{
    return groove_storage::StorageStatus::forCondition(
        groove_storage::StorageCondition::kStorageInvariant, "deleteDataset is not implemented");
}

tempo_utils::Result<std::vector<std::string>>
StorageSupervisor::putData(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame)
{
    TU_ASSERT (datasetUrl.isValid());
    TU_ASSERT (!modelId.empty());
    TU_ASSERT (frame != nullptr);

    auto dataset = getDataset(datasetUrl);
    if (dataset == nullptr)
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "missing dataset");
    if (dataset->isImmutable())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "dataset is immutable");

    TU_LOG_INFO << "updating model " << modelId << " in " << datasetUrl
        << " with frame containing " << frame->numVectors()
        << " and " << frame->getSize() << " rows";

    std::vector<std::string> failedVectors;
    auto status = m_db->updateModel(datasetUrl, modelId, frame, &failedVectors);
    if (status.notOk())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "failed to update model");
    return failedVectors;
}
