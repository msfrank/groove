#ifndef GROOVE_STORAGE_REMOTE_COLLECTION_H
#define GROOVE_STORAGE_REMOTE_COLLECTION_H

#include <absl/synchronization/notification.h>

#include <tempo_utils/url.h>

#include "storage_client.h"
#include "storage_result.h"

namespace groove_storage {

    enum class RemoteCollectionState {
        Pending,
        Active,
        Closed,
    };

    class RemoteCollection : public MountCollectionHandler {

    public:
        RemoteCollection(std::shared_ptr<StorageClient> client, tu_uint64 lsn);
        ~RemoteCollection();

        StorageStatus waitUntilActive();

        tempo_utils::Url getCollectionUrl() const;
        RemoteCollectionState getState() const;

        tempo_utils::Result<tempo_utils::Url> createDataset(
            const tempo_utils::Url &containerUrl,
            const std::string &datasetName,
            const groove_model::GrooveSchema &schema);
        tempo_utils::Result<std::vector<std::string>> putData(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseFrame> frame);
        tempo_utils::Status unmount();

        void collectionCreated(const tempo_utils::Url &collectionUrl) override;
        void collectionOpened(const tempo_utils::Url &collectionUrl) override;
        void containerAdded(const tempo_utils::Url &containerUrl) override;
        void containerRemoved(const tempo_utils::Url &containerUrl) override;
        void datasetAdded(const tempo_utils::Url &datasetUrl) override;
        void datasetRemoved(const tempo_utils::Url &datasetUrl) override;
        void mountCollectionFinished(
            const tempo_utils::Url &collectionUrl,
            const tempo_utils::Status &status) override;

    private:
        std::shared_ptr<StorageClient> m_client;
        tu_uint64 m_lsn;

        absl::Mutex *m_lock;
        tempo_utils::Url m_collectionUrl ABSL_GUARDED_BY(m_lock);
        RemoteCollectionState m_state ABSL_GUARDED_BY(m_lock);
        absl::Notification *m_notification;
    };
}

#endif // GROOVE_STORAGE_REMOTE_COLLECTION_H