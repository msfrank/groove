#ifndef GROOVE_STORAGE_SYNC_PROVIDER_H
#define GROOVE_STORAGE_SYNC_PROVIDER_H

#include <groove_model/groove_database.h>
#include <tempo_utils/url.h>
#include <grpcpp/security/credentials.h>

#include "dataset_synchronizer.h"
#include "syncing_client.h"
#include "storage_result.h"

namespace groove_storage {

    class SyncProvider {

    public:
        explicit SyncProvider(std::shared_ptr<groove_model::GrooveDatabase> db);

        tempo_utils::Result<std::shared_ptr<DatasetSynchronizer>>
        getSynchronizer(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<grpc::ChannelCredentials> credentials = {},
            const tempo_utils::Url &endpointUrl = {},
            const std::string &endpointServerName = {});

    private:
        absl::Mutex m_lock;
        std::shared_ptr<groove_model::GrooveDatabase> m_db;
        absl::flat_hash_map<tempo_utils::Url, std::shared_ptr<DatasetSynchronizer>> m_datasetSynchronizers;
        absl::flat_hash_map<tempo_utils::Url, std::shared_ptr<SyncingClient>> m_endpointClients;
    };
}

#endif // GROOVE_STORAGE_SYNC_PROVIDER_H