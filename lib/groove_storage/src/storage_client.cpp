
#include <groove_data/table_utils.h>
#include <groove_storage/storage_client.h>
#include <tempo_config/config_serde.h>

groove_storage::StorageClient::StorageClient(
    const tempo_utils::Url &endpointUrl,
    std::shared_ptr<grpc::ChannelCredentials> credentials,
    const std::string &endpointServerName)
    : m_endpointUrl(endpointUrl),
      m_credentials(credentials),
      m_endpointServerName(endpointServerName),
      m_lastLsn(0)
{
    TU_ASSERT (m_endpointUrl.isValid());
    TU_ASSERT (m_credentials != nullptr);
}

tempo_utils::Status
groove_storage::StorageClient::connect()
{
    absl::MutexLock lock(&m_lock);

    if (m_stub)
        return StorageStatus::forCondition(
            StorageCondition::kStorageInvariant, "storage client is already connected");

    // construct the client
    grpc::ChannelArguments channelArguments;
    if (!m_endpointServerName.empty())
        channelArguments.SetSslTargetNameOverride(m_endpointServerName);
    m_channel = grpc::CreateCustomChannel(m_endpointUrl.toString(), m_credentials, channelArguments);
    m_stub = groove_mount::MountService::NewStub(m_channel);

    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::shutdown()
{
    absl::MutexLock lock(&m_lock);
    if (m_stub) {
        TU_LOG_INFO << "shutting down communication with " << m_endpointUrl;
        m_stub.reset();
    }
    return StorageStatus::ok();
}

tempo_utils::Result<groove_storage::StorageIdentity>
groove_storage::StorageClient::identify()
{
    grpc::ClientContext context;
    groove_mount::IdentifyAgentRequest request;
    groove_mount::IdentifyAgentResult result;

    auto status = m_stub->IdentifyAgent(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "Identify failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    StorageIdentity identity;
    identity.agentName = result.agent_name();
    identity.uptimeMillis = result.uptime_millis();
    return identity;
}

tempo_utils::Result<tu_uint64>
groove_storage::StorageClient::declareCollection(
    const std::string &collectionName,
    const tempo_config::ConfigMap &configMap,
    MountMode mode)
{
    absl::MutexLock locker(&m_lock);
    auto lsn = ++m_lastLsn;
    auto reactor = new MountCollectionReactor(shared_from_this(), lsn, collectionName, configMap, mode);
    m_reactors[lsn] = reactor;
    return lsn;
}

tempo_utils::Status
groove_storage::StorageClient::mountCollection(tu_uint64 lsn, std::shared_ptr<MountCollectionHandler> handler)
{
    absl::MutexLock locker(&m_lock);
    if (!m_reactors.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    auto *reactor = m_reactors.at(lsn);
    return reactor->start(m_stub.get(), handler);
}

tempo_utils::Status
groove_storage::StorageClient::registerCollection(tu_uint64 lsn, const tempo_utils::Url &collectionUrl)
{
    absl::MutexLock locker(&m_lock);
    if (!m_reactors.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    if (m_collectionToLsn.contains(collectionUrl) || m_lsnToCollection.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "collection for lsn is already registered");
    m_collectionToLsn[collectionUrl] = lsn;
    m_lsnToCollection[lsn] = collectionUrl;
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::unmountCollection(tu_uint64 lsn)
{
    absl::MutexLock locker(&m_lock);
    if (!m_reactors.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    auto *reactor = m_reactors.at(lsn);
    return reactor->cancel();
}

tempo_utils::Status
groove_storage::StorageClient::releaseCollection(tu_uint64 lsn)
{
    absl::MutexLock locker(&m_lock);

    if (!m_reactors.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    if (!m_lsnToCollection.contains(lsn))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    auto collectionUrl = m_lsnToCollection.at(lsn);
    if (!m_collectionToLsn.contains(collectionUrl))
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "missing collection for lsn");
    m_lsnToCollection.erase(lsn);
    m_collectionToLsn.erase(collectionUrl);

    auto *reactor = m_reactors.at(lsn);
    m_reactors.erase(lsn);
    delete reactor;

    return StorageStatus::ok();
}

tempo_utils::Result<tempo_utils::Url>
groove_storage::StorageClient::createContainer(
    const tempo_utils::Url &rootOrContainerUrl,
    const std::string &storageName)
{
    grpc::ClientContext context;
    groove_mount::CreateContainerRequest request;
    groove_mount::CreateContainerResult result;

    request.set_container_uri(rootOrContainerUrl.toString());
    request.set_name(storageName);

    auto status = m_stub->CreateContainer(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "CreateContainer failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    return tempo_utils::Url::fromString(result.container_uri());
}

tempo_utils::Status
groove_storage::StorageClient::createContainer(
    const tempo_utils::Url &rootOrContainerUrl,
    const std::string &storageName,
    std::shared_ptr<CreateContainerHandler> handler)
{
    new CreateContainerReactor(m_stub.get(), rootOrContainerUrl, storageName, handler);
    return StorageStatus::ok();
}

tempo_utils::Result<tempo_utils::Url>
groove_storage::StorageClient::createDataset(
    const tempo_utils::Url &containerUrl,
    const std::string &datasetName,
    const groove_model::GrooveSchema &schema)
{
    grpc::ClientContext context;
    groove_mount::CreateDatasetRequest request;
    groove_mount::CreateDatasetResult result;

    request.set_container_uri(containerUrl.toString());
    request.set_name(datasetName);
    auto schemaBytes = schema.bytesView();
    request.set_schema_bytes(std::string((const char *) schemaBytes.data(), schemaBytes.size()));

    auto status = m_stub->CreateDataset(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "CreateDataset failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    return tempo_utils::Url::fromString(result.dataset_uri());
}

tempo_utils::Status
groove_storage::StorageClient::createDataset(
    const tempo_utils::Url &containerUrl,
    const std::string &datasetName,
    const groove_model::GrooveSchema &schema,
    std::shared_ptr<CreateDatasetHandler> handler)
{
    new CreateDatasetReactor(m_stub.get(), containerUrl, datasetName, schema, handler);
    return StorageStatus::ok();
}

tempo_utils::Result<std::vector<std::string>>
groove_storage::StorageClient::putData(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame)
{
    grpc::ClientContext context;
    groove_mount::PutDataRequest request;
    groove_mount::PutDataResult result;

    request.set_dataset_uri(datasetUrl.toString());
    request.set_model_id(modelId);

    // use the key index from the frame
    request.set_key_index(frame->getKeyFieldIndex());

    // add value field for each vector in the frame
    for (auto iterator = frame->vectorsBegin(); iterator != frame->vectorsEnd(); iterator++) {
        const auto vector = iterator->second;
        auto *valueField = request.add_value_fields();
        valueField->set_val_index(vector->getValFieldIndex());
        valueField->set_fid_index(vector->getFidFieldIndex());
    }

    // serialize table to buffer
    auto makeBufferResult = groove_data::make_buffer(frame->getUnderlyingTable());
    if (makeBufferResult.isStatus())
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, makeBufferResult.getStatus().getMessage());
    auto buffer = makeBufferResult.getResult();

    // NOTE: we hold buffer reference for lifetime of this string so its ok to not copy
    std::string frameBytes((const char *) buffer->data(), buffer->size());

    switch (frame->getFrameType()) {
        case groove_data::DataFrameType::FRAME_TYPE_CATEGORY:
            request.set_cat(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_DOUBLE:
            request.set_dbl(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_INT64:
            request.set_i64(std::move(frameBytes));
            break;
        default:
            TU_UNREACHABLE();
    }

    auto status = m_stub->PutData(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "PutData failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    std::vector<std::string> failedVectors;
    for (int i = 0; i < result.failed_vectors_size(); i++) {
        failedVectors.push_back(result.failed_vectors(i));
    }
    return failedVectors;
}

tempo_utils::Status
groove_storage::StorageClient::putData(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame,
    std::shared_ptr<PutDataHandler> handler)
{
    // serialize table to buffer
    auto makeBufferResult = groove_data::make_buffer(frame->getUnderlyingTable());
    if (makeBufferResult.isStatus())
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, makeBufferResult.getStatus().getMessage());
    auto buffer = makeBufferResult.getResult();
    new PutDataReactor(m_stub.get(), datasetUrl, modelId, frame, buffer, handler);
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::deleteContainer(const tempo_utils::Url &containerUrl)
{
    grpc::ClientContext context;
    groove_mount::DeleteContainerRequest request;
    groove_mount::DeleteContainerResult result;

    request.set_container_uri(containerUrl.toString());
    auto status = m_stub->DeleteContainer(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "DeleteContainer failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::deleteContainer(
    const tempo_utils::Url &containerUrl,
    std::shared_ptr<DeleteContainerHandler> handler)
{
    new DeleteContainerReactor(m_stub.get(), containerUrl, handler);
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::deleteDataset(const tempo_utils::Url &datasetUrl)
{
    grpc::ClientContext context;
    groove_mount::DeleteDatasetRequest request;
    groove_mount::DeleteDatasetResult result;

    request.set_dataset_uri(datasetUrl.toString());
    auto status = m_stub->DeleteDataset(&context, request, &result);
    if (!status.ok()) {
        TU_LOG_ERROR << "DeleteDataset failed: " << status.error_message() << " (" << status.error_details() << ")";
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message());
    }
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::StorageClient::deleteDataset(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<DeleteDatasetHandler> handler)
{
    new DeleteDatasetReactor(m_stub.get(), datasetUrl, handler);
    return StorageStatus::ok();
}

tempo_utils::Result<std::shared_ptr<groove_storage::StorageClient>>
groove_storage::StorageClient::create(
    const tempo_utils::Url &endpointUrl,
    std::shared_ptr<grpc::ChannelCredentials> credentials,
    const std::string &endpointServerName)
{
    return std::shared_ptr<StorageClient>(
        new StorageClient(endpointUrl, credentials, endpointServerName));
}

groove_storage::MountCollectionReactor::MountCollectionReactor(
    std::shared_ptr<StorageClient> client,
    tu_uint64 lsn,
    const std::string &rootName,
    const tempo_config::ConfigMap &configMap,
    MountMode mode)
    : m_client(client),
      m_lsn(lsn)
{
    TU_ASSERT (m_client != nullptr);
    TU_ASSERT (!rootName.empty());

    m_request.set_name(rootName);
    std::string config;
    tempo_config::write_config_string(configMap, config);
    m_request.set_config_hash(config);
    switch (mode) {
        case MountMode::Open:
            m_request.set_mode(groove_mount::MountCollectionRequest_MountMode_Open);
            break;
        case MountMode::OpenOrCreate:
            m_request.set_mode(groove_mount::MountCollectionRequest_MountMode_OpenOrCreate);
            break;
        case MountMode::CreateOnly:
            m_request.set_mode(groove_mount::MountCollectionRequest_MountMode_CreateOnly);
            break;
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Status
groove_storage::MountCollectionReactor::start(
    groove_mount::MountService::Stub *stub,
    std::shared_ptr<MountCollectionHandler> handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (handler != nullptr);

    if (m_handler != nullptr)
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "reactor has already started");
    m_handler = handler;
    stub->async()->MountCollection(&m_context, &m_request, this);
    StartCall();
    TU_LOG_INFO << "mounting collection for lsn " << m_lsn;
    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::MountCollectionReactor::cancel()
{
    m_context.TryCancel();
    return StorageStatus::ok();
}

void
groove_storage::MountCollectionReactor::OnReadInitialMetadataDone(bool ok)
{
    TU_LOG_ERROR_IF (!ok) << "failed to read initial metadata";
    StartRead(&m_incoming);
}

void
groove_storage::MountCollectionReactor::OnReadDone(bool ok)
{
    if (!ok) {
        TU_LOG_ERROR << "read failed";
        return;
    }

    auto url = tempo_utils::Url::fromString(m_incoming.uri());
    switch (m_incoming.type()) {
        case groove_mount::CollectionChange_CollectionChangeType_Created:
            m_collectionUrl = url;
            m_handler->collectionCreated(m_collectionUrl);
            break;
        case groove_mount::CollectionChange_CollectionChangeType_Opened:
            m_collectionUrl = url;
            m_handler->collectionOpened(m_collectionUrl);
            break;
        case groove_mount::CollectionChange_CollectionChangeType_AddedContainer:
            m_handler->containerAdded(url);
            break;
        case groove_mount::CollectionChange_CollectionChangeType_RemovedContainer:
            m_handler->containerRemoved(url);
            break;
        case groove_mount::CollectionChange_CollectionChangeType_AddedDataset:
            m_handler->datasetAdded(url);
            break;
        case groove_mount::CollectionChange_CollectionChangeType_RemovedDataset:
            m_handler->datasetRemoved(url);
            break;
        default:
            TU_LOG_ERROR << "ignoring unknown change type";
            break;
    }

    m_incoming.Clear();
    StartRead(&m_incoming);
}

void
groove_storage::MountCollectionReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        m_handler->mountCollectionFinished(m_collectionUrl, StorageStatus::ok());
    } else {
        m_handler->mountCollectionFinished({}, StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    auto releaseStatus = m_client->releaseCollection(m_lsn);
    TU_LOG_ERROR_IF(releaseStatus.notOk())
        << "MountCollection failed: " << status.error_message()
        << " (" << status.error_details() << ")";
}

groove_storage::CollectionUnmounter::CollectionUnmounter(std::shared_ptr<StorageClient> client, tu_uint64 lsn)
    : m_client(client),
      m_lsn(lsn)
{
    TU_ASSERT (m_client != nullptr);
    TU_ASSERT (m_lsn > 0);
}

tempo_utils::Status
groove_storage::CollectionUnmounter::unmount()
{
    return m_client->unmountCollection(m_lsn);
}

groove_storage::CreateContainerReactor::CreateContainerReactor(
    groove_mount::MountService::Stub *stub,
    const tempo_utils::Url &rootOrContainerUrl,
    const std::string &storageName,
    std::shared_ptr<CreateContainerHandler> handler)
    : m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (!storageName.empty());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_container_uri(rootOrContainerUrl.toString());
    m_request.set_name(storageName);

    stub->async()->CreateContainer(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::CreateContainerReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        auto containerUrl = tempo_utils::Url::fromString(m_result.container_uri());
        m_handler->createContainerFinished(containerUrl, StorageStatus::ok());
    } else {
        TU_LOG_ERROR << "CreateContainer failed: " << status.error_message() << " (" << status.error_details() << ")";
        m_handler->createContainerFinished({}, StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::CreateDatasetReactor::CreateDatasetReactor(
    groove_mount::MountService::Stub *stub,
    const tempo_utils::Url &containerUrl,
    const std::string &datasetName,
    const groove_model::GrooveSchema &schema,
    std::shared_ptr<CreateDatasetHandler> handler)
    : m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (containerUrl.isValid());
    TU_ASSERT (!datasetName.empty());
    TU_ASSERT (!schema.isValid());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_container_uri(containerUrl.toString());
    m_request.set_name(datasetName);
    auto schemaBytes = schema.bytesView();
    m_request.set_schema_bytes(std::string((const char *) schemaBytes.data(), schemaBytes.size()));

    stub->async()->CreateDataset(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::CreateDatasetReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        auto datasetUrl = tempo_utils::Url::fromString(m_result.dataset_uri());
        m_handler->createDatasetFinished(datasetUrl, StorageStatus::ok());
    } else {
        TU_LOG_ERROR << "CreateDataset failed: " << status.error_message() << " (" << status.error_details() << ")";
        m_handler->createDatasetFinished({}, StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::PutDataReactor::PutDataReactor(
    groove_mount::MountService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame,
    std::shared_ptr<const arrow::Buffer> buffer,
    std::shared_ptr<PutDataHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_frame(frame),
      m_buffer(buffer),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_frame != nullptr);
    TU_ASSERT (m_buffer != nullptr);
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(m_datasetUrl.toString());
    m_request.set_model_id(modelId);

    // use the key index from the frame
    m_request.set_key_index(frame->getKeyFieldIndex());

    // add value field for each vector in the frame
    for (auto iterator = frame->vectorsBegin(); iterator != frame->vectorsEnd(); iterator++) {
        const auto vector = iterator->second;
        auto *valueField = m_request.add_value_fields();
        valueField->set_val_index(vector->getValFieldIndex());
        valueField->set_fid_index(vector->getFidFieldIndex());
    }

    // NOTE: we hold buffer reference for lifetime of this string so its ok to not copy
    std::string frameBytes((const char *) buffer->data(), buffer->size());

    switch (frame->getFrameType()) {
        case groove_data::DataFrameType::FRAME_TYPE_CATEGORY:
            m_request.set_cat(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_DOUBLE:
            m_request.set_dbl(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_INT64:
            m_request.set_i64(std::move(frameBytes));
            break;
        default:
            TU_UNREACHABLE();
    }

    stub->async()->PutData(&m_context, &m_request, &m_result, this);
    StartCall();
}

void groove_storage::PutDataReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        std::vector<std::string> failedVectors;
        for (int i = 0; i < m_result.failed_vectors_size(); i++) {
            failedVectors.push_back(m_result.failed_vectors(i));
        }
        m_handler->putDataFinished(m_datasetUrl, m_modelId, failedVectors, StorageStatus::ok());
    } else {
        TU_LOG_ERROR << "PutData failed: " << status.error_message() << " (" << status.error_details() << ")";
        m_handler->putDataFinished(m_datasetUrl, m_modelId, {},
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::DeleteContainerReactor::DeleteContainerReactor(
    groove_mount::MountService::Stub *stub,
    const tempo_utils::Url &containerUrl,
    std::shared_ptr<DeleteContainerHandler> handler)
    : m_containerUrl(containerUrl),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_containerUrl.isValid());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_container_uri(m_containerUrl.toString());
    stub->async()->DeleteContainer(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::DeleteContainerReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        m_handler->deleteContainerFinished(m_containerUrl, StorageStatus::ok());
    } else {
        TU_LOG_ERROR << "DeleteContainer failed: " << status.error_message() << " (" << status.error_details() << ")";
        m_handler->deleteContainerFinished({}, StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::DeleteDatasetReactor::DeleteDatasetReactor(
    groove_mount::MountService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<DeleteDatasetHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(m_datasetUrl.toString());
    stub->async()->DeleteDataset(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::DeleteDatasetReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        m_handler->deleteDatasetFinished(m_datasetUrl, StorageStatus::ok());
    } else {
        TU_LOG_ERROR << "DeleteDataset failed: " << status.error_message() << " (" << status.error_details() << ")";
        m_handler->deleteDatasetFinished({},
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

void
groove_storage::MountCollectionHandler::collectionCreated(const tempo_utils::Url &collectionUrl)
{
}

void
groove_storage::MountCollectionHandler::collectionOpened(const tempo_utils::Url &collectionUrl)
{
}

void
groove_storage::MountCollectionHandler::containerAdded(const tempo_utils::Url &containerUrl)
{
}

void
groove_storage::MountCollectionHandler::containerRemoved(const tempo_utils::Url &containerUrl)
{
}

void
groove_storage::MountCollectionHandler::datasetAdded(const tempo_utils::Url &datasetUrl)
{
}

void
groove_storage::MountCollectionHandler::datasetRemoved(const tempo_utils::Url &datasetUrl)
{
}

void
groove_storage::MountCollectionHandler::mountCollectionFinished(
    const tempo_utils::Url &collectionUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::CreateContainerHandler::createContainerFinished(
    const tempo_utils::Url &containerUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::CreateDatasetHandler::createDatasetFinished(
    const tempo_utils::Url &datasetUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::PutDataHandler::putDataFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::vector<std::string> &failedVectors,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::RemoveDataHandler::removeDataFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::DeleteContainerHandler::deleteContainerFinished(
    const tempo_utils::Url &containerUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::DeleteDatasetHandler::deleteDatasetFinished(
    const tempo_utils::Url &datasetUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}
