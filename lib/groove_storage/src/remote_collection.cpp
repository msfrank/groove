
#include <groove_storage/remote_collection.h>

groove_storage::RemoteCollection::RemoteCollection(std::shared_ptr<StorageClient> client, tu_uint64 lsn)
    : m_client(client),
      m_lsn(lsn),
      m_state(RemoteCollectionState::Pending)
{
    TU_ASSERT (m_client != nullptr);
    TU_ASSERT (m_lsn > 0);
    m_lock = new absl::Mutex();
}

groove_storage::RemoteCollection::~RemoteCollection()
{
    delete m_lock;
}

groove_storage::StorageStatus
groove_storage::RemoteCollection::waitUntilActive()
{
    {
        absl::MutexLock locker(m_lock);
        switch (m_state) {
            case RemoteCollectionState::Pending:
                break;
            case RemoteCollectionState::Active:
                return StorageStatus::ok();
            case RemoteCollectionState::Closed:
                return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "collection is closed");
            default:
                return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "invalid collection state");
        }
        if (m_notification != nullptr)
            return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "already waiting on collection");
        m_notification = new absl::Notification();
    }

    TU_LOG_INFO << "waiting for remote collection to become active";
    m_notification->WaitForNotification();

    absl::MutexLock locker(m_lock);
    delete m_notification;
    m_notification = nullptr;
    switch (m_state) {
        case RemoteCollectionState::Active:
            return StorageStatus::ok();
        case RemoteCollectionState::Closed:
            return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "collection is closed");
        case RemoteCollectionState::Pending:
        default:
            return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "invalid collection state");
    }
}

tempo_utils::Url
groove_storage::RemoteCollection::getCollectionUrl() const
{
    absl::MutexLock locker(m_lock);
    return m_collectionUrl;
}

groove_storage::RemoteCollectionState
groove_storage::RemoteCollection::getState() const
{
    absl::MutexLock locker(m_lock);
    return m_state;
}

tempo_utils::Result<tempo_utils::Url>
groove_storage::RemoteCollection::createDataset(
    const tempo_utils::Url &containerUrl,
    const std::string &datasetName,
    const groove_model::GrooveSchema &schema)
{
    absl::MutexLock locker(m_lock);
    return m_client->createDataset(containerUrl, datasetName, schema);
}

tempo_utils::Result<std::vector<std::string>>
groove_storage::RemoteCollection::putData(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame)
{
    absl::MutexLock locker(m_lock);
    return m_client->putData(datasetUrl, modelId, frame);
}

tempo_utils::Status
groove_storage::RemoteCollection::unmount()
{
    absl::MutexLock locker(m_lock);
    switch (m_state) {
        case RemoteCollectionState::Pending:
        case RemoteCollectionState::Active:
            return m_client->unmountCollection(m_lsn);
        case RemoteCollectionState::Closed:
            return StorageStatus::ok();
        default:
            return StorageStatus::forCondition(
                StorageCondition::kStorageInvariant, "invalid container state");
    }
}

void
groove_storage::RemoteCollection::collectionCreated(const tempo_utils::Url &collectionUrl)
{
    absl::MutexLock locker(m_lock);
    m_state = RemoteCollectionState::Active;
    m_collectionUrl = collectionUrl;
    if (m_notification != nullptr) {
        m_notification->Notify();
    }
}

void
groove_storage::RemoteCollection::collectionOpened(const tempo_utils::Url &collectionUrl)
{
    absl::MutexLock locker(m_lock);
    m_state = RemoteCollectionState::Active;
    m_collectionUrl = collectionUrl;
    if (m_notification != nullptr) {
        m_notification->Notify();
    }
}

void
groove_storage::RemoteCollection::containerAdded(const tempo_utils::Url &containerUrl)
{
    TU_LOG_INFO << "added container " << containerUrl;
}

void
groove_storage::RemoteCollection::containerRemoved(const tempo_utils::Url &containerUrl)
{
    TU_LOG_INFO << "removed container " << containerUrl;
}

void
groove_storage::RemoteCollection::datasetAdded(const tempo_utils::Url &datasetUrl)
{
    TU_LOG_INFO << "added dataset " << datasetUrl;
}

void
groove_storage::RemoteCollection::datasetRemoved(const tempo_utils::Url &datasetUrl)
{
    TU_LOG_INFO << "removed dataset " << datasetUrl;
}

void
groove_storage::RemoteCollection::mountCollectionFinished(
    const tempo_utils::Url &collectionUrl,
    const tempo_utils::Status &status)
{
    absl::MutexLock locker(m_lock);
    m_state = RemoteCollectionState::Closed;
    if (m_notification != nullptr) {
        m_notification->Notify();
    }
}