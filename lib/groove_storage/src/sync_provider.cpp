
#include <groove_storage/sync_provider.h>

groove_storage::SyncProvider::SyncProvider(std::shared_ptr <groove_model::GrooveDatabase> db)
    : m_db(db)
{
    TU_ASSERT (m_db != nullptr);
}

tempo_utils::Result<std::shared_ptr<groove_storage::DatasetSynchronizer>>
groove_storage::SyncProvider::getSynchronizer(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<grpc::ChannelCredentials> credentials,
    const tempo_utils::Url &endpointUrl,
    const std::string &endpointServerName)
{
    absl::MutexLock locker(&m_lock);

    // if a synchronizer already exists then return it
    if (m_datasetSynchronizers.contains(datasetUrl))
        return m_datasetSynchronizers.at(datasetUrl);

    if (!endpointUrl.isValid())
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing endpoint uri");

    // get existing cached client, or create client if one doesn't exist
    std::shared_ptr<SyncingClient> client;
    if (!m_endpointClients.contains(endpointUrl)) {
        auto createClientResult = SyncingClient::create(endpointUrl, credentials, endpointServerName);
        if (createClientResult.isStatus())
            return createClientResult.getStatus();
        client = createClientResult.getResult();
        auto status = client->connect();
        if (status.notOk())
            return status;
        m_endpointClients[endpointUrl] = client;
    } else {
        client = m_endpointClients.at(endpointUrl);
    }

    // create the synchronizer and return it
    auto createSynchronizerResult = DatasetSynchronizer::create(m_db, datasetUrl, client);
    if (createSynchronizerResult.isStatus())
        return createSynchronizerResult.getStatus();
    auto synchronizer = createSynchronizerResult.getResult();
    m_datasetSynchronizers[datasetUrl] = synchronizer;

    return synchronizer;
}
