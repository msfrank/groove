
#include <boost/uuid/uuid_io.hpp>
#include <uv.h>

#include <groove_agent/mount_service.h>
#include <groove_data/category_frame.h>
#include <groove_data/double_frame.h>
#include <groove_data/int64_frame.h>
#include <groove_model/groove_database.h>
#include <tempo_config/config_serde.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/memory_bytes.h>

MountService::MountService(
    const tempo_utils::Url &listenerUrl,
    StorageSupervisor *supervisor,
    std::string_view agentName)
    : m_listenerUrl(listenerUrl),
      m_supervisor(supervisor),
      m_agentName(agentName)
{
    TU_ASSERT (m_listenerUrl.isValid());
    TU_ASSERT (m_supervisor != nullptr);
    uv_timeval64_t tv;
    uv_gettimeofday(&tv);
    m_uptime = tv.tv_sec;
}

grpc::ServerUnaryReactor *
MountService::MountService::IdentifyAgent(
    grpc::CallbackServerContext *context,
    const groove_mount::IdentifyAgentRequest *request,
    groove_mount::IdentifyAgentResult *response)
{
    uv_timeval64_t tv;
    uv_gettimeofday(&tv);
    response->set_agent_name(m_agentName);
    response->set_uptime_millis(tv.tv_sec - m_uptime);
    auto *reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

grpc::ServerWriteReactor<groove_mount::CollectionChange> *
MountService::MountCollection(
    grpc::CallbackServerContext *context,
    const groove_mount::MountCollectionRequest *request)
{
    TU_LOG_INFO << "MountCollection request: " << request->DebugString();

    if (request->mode() == groove_mount::MountCollectionRequest_MountMode_Ephemeral
      && request->name().empty()) {
        auto *reactor = new CollectionChangeWriter();
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing required name"));
        return reactor;
    }
    auto collectionName = request->name();

    auto parseConfigResult = tempo_config::read_config_string(request->config_hash());
    if (parseConfigResult.isStatus()) {
        auto *reactor = new CollectionChangeWriter();
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT,
            std::string(parseConfigResult.getStatus().getMessage())));
        return reactor;
    }
    auto config = parseConfigResult.getResult();

    if (config.getNodeType() != tempo_config::ConfigNodeType::kMap) {
        auto *reactor = new CollectionChangeWriter();
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid config hash"));
        return reactor;
    }
    auto configMap = config.toMap();

    //
    std::shared_ptr<StorageCollection> collection;
    bool created;
    switch (request->mode()) {
        case groove_mount::MountCollectionRequest_MountMode_Open: {
            collection = m_supervisor->getCollection(collectionName);
            if (collection == nullptr) {
                auto *reactor = new CollectionChangeWriter();
                reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing collection"));
                return reactor;
            }
            created = false;
            break;
        }
        case groove_mount::MountCollectionRequest_MountMode_OpenOrCreate: {
            auto getOrCreateResult = m_supervisor->getOrCreateCollection(collectionName);
            if (getOrCreateResult.isStatus()) {
                auto *reactor = new CollectionChangeWriter();
                reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid collection"));
                return reactor;
            }
            auto result = getOrCreateResult.getResult();
            collection = result.first;
            created = result.second;
            break;
        }
        case groove_mount::MountCollectionRequest_MountMode_CreateOnly: {
            auto createResult = m_supervisor->createCollection(collectionName);
            if (createResult.isStatus()) {
                auto *reactor = new CollectionChangeWriter();
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to create collection"));
                return reactor;
            }
            collection = createResult.getResult();
            created = true;
            break;
        }
        case groove_mount::MountCollectionRequest_MountMode_Ephemeral: {
            auto createEphemeralResult = m_supervisor->createEphemeralCollection(collectionName);
            if (createEphemeralResult.isStatus()) {
                auto *reactor = new CollectionChangeWriter();
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to create collection"));
                return reactor;
            }
            collection = createEphemeralResult.getResult();
            created = true;
            break;
        }
        default:
            break;
    }

    // construct the reactor
    auto *reactor = new CollectionChangeWriter(collection);
    auto attachStatus = reactor->attach();
    if (attachStatus.notOk()) {
        reactor->Finish(grpc::Status( grpc::StatusCode::INTERNAL,
            "failed to attach watcher to collection", std::string(attachStatus.getMessage())));
        return reactor;
    }

    // notify the client that collection was opened or created
    reactor->collectionMounted(created);

    TU_LOG_INFO << "returning reactor";

    return reactor;
}

grpc::ServerUnaryReactor *
MountService::CreateDataset(
    grpc::CallbackServerContext *context,
    const groove_mount::CreateDatasetRequest *request,
    groove_mount::CreateDatasetResult *response)
{
    auto *reactor = context->DefaultReactor();

    //
    if (request->container_uri().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing container uri"));
        return reactor;
    }
    auto containerUrl = tempo_utils::Url::fromString(request->container_uri());
    if (!containerUrl.isValid()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "container url is invalid"));
        return reactor;
    }

    //
    if (request->name().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing name"));
        return reactor;
    }
    auto datasetName = request->name();

    //
    if (request->schema_bytes().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing schema"));
        return reactor;
    }
    auto schemaBytes = tempo_utils::MemoryBytes::copy(request->schema_bytes());

    //
    if (!groove_model::GrooveSchema::verify(schemaBytes->getSpan())) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid schema"));
        return reactor;
    }

    auto containerOrigin = containerUrl.toOrigin();
    auto containerPath = containerUrl.toPath();
    auto datasetPath = containerPath.traverse(tempo_utils::UrlPathPart(datasetName));

    auto datasetUrl = tempo_utils::Url::fromOrigin(containerOrigin.toString(), datasetPath.toString());
    //auto datasetUrl = containerUrl / tempo_utils::Url::fromString(absl::StrCat(containerUrl.getPath(), "/test"));

    auto schema = groove_model::GrooveSchema(schemaBytes);

    auto createDatasetResult = m_supervisor->createDataset(datasetUrl, schema);
    if (createDatasetResult.isStatus()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND,
            "failed to create dataset", std::string(createDatasetResult.getStatus().getMessage())));
        return reactor;
    }

    response->set_dataset_uri(datasetUrl.toString());
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

grpc::ServerUnaryReactor *
MountService::PutData(
    grpc::CallbackServerContext *context,
    const groove_mount::PutDataRequest *request,
    groove_mount::PutDataResult *response)
{
    auto *reactor = context->DefaultReactor();

    //
    if (request->dataset_uri().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing dataset uri"));
        return reactor;
    }
    auto datasetUrl = tempo_utils::Url::fromString(request->dataset_uri());
    if (!datasetUrl.isValid()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid dataset uri"));
        return reactor;
    }

    //
    if (request->model_id().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing model id"));
        return reactor;
    }
    auto modelId = request->model_id();

    //
    std::vector<std::pair<int,int>> valueColumns;
    for (const auto &valueField : request->value_fields()) {
        valueColumns.push_back(std::pair<int,int>{valueField.val_index(), valueField.fid_index()});
    }

    //
    auto keyFieldIndex = request->key_index();

    std::shared_ptr<groove_data::BaseFrame> frame;
    switch (request->frame_case()) {
        case groove_mount::PutDataRequest::kCat: {
            auto makeTableResult = groove_data::make_table(std::make_shared<const std::string>(request->cat()));
            if (makeTableResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse category frame"));
                return reactor;
            }
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::CategoryFrame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse category frame"));
                return reactor;
            }
            frame = createFrameResult.getResult();
            break;
        }
        case groove_mount::PutDataRequest::kDbl: {
            auto makeTableResult = groove_data::make_table(std::make_shared<const std::string>(request->dbl()));
            if (makeTableResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse double frame"));
                return reactor;
            }
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::DoubleFrame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse double frame"));
                return reactor;
            }
            frame = createFrameResult.getResult();
            break;
        }
        case groove_mount::PutDataRequest::kI64: {
            auto makeTableResult = groove_data::make_table(std::make_shared<const std::string>(request->i64()));
            if (makeTableResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse int64 frame"));
                return reactor;
            }
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::Int64Frame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus()) {
                reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to parse int64 frame"));
                return reactor;
            }
            frame = createFrameResult.getResult();
            break;
        }
        default:
            reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "unknown frame type"));
            return reactor;
    }

    //
    auto putDataResult = m_supervisor->putData(datasetUrl, modelId, frame);
    if (putDataResult.isStatus()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to put data"));
        return reactor;
    }
    auto failedVectors = putDataResult.getResult();

    //
    for (const auto &failedVector : failedVectors) {
        response->add_failed_vectors(std::move(failedVector));
    }
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

CollectionChangeWriter::CollectionChangeWriter()
    : m_watchkey(0),
      m_head(nullptr),
      m_tail(nullptr)
{
}

CollectionChangeWriter::CollectionChangeWriter(std::shared_ptr<StorageCollection> collection)
    : m_collection(collection),
      m_watchkey(0),
      m_head(nullptr),
      m_tail(nullptr)
{
    TU_ASSERT (m_collection != nullptr);
}

CollectionChangeWriter::~CollectionChangeWriter()
{
    TU_LOG_ERROR_IF(m_watchkey >= 0) << "watcher was not detached from collection";
    auto *curr = m_head;
    while (curr != nullptr) {
        auto *next = curr->next;
        delete curr;
        curr = next;
    }
}

tempo_utils::Status
CollectionChangeWriter::attach()
{
    absl::MutexLock locker(&m_lock);

    if (m_watchkey > 0)
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "watcher is already attached to collection");
    auto attachResult = m_collection->attachWatcher(this);
    if (attachResult.isStatus())
        return attachResult.getStatus();
    m_watchkey = attachResult.getResult();
    return groove_storage::StorageStatus::ok();
}

tempo_utils::Status
CollectionChangeWriter::detach()
{
    if (m_watchkey == 0)
        return groove_storage::StorageStatus::ok();
    auto status = m_collection->detachWatcher(m_watchkey);
    if (status.notOk())
        return status;
    m_watchkey = 0;
    return groove_storage::StorageStatus::ok();
}

void
CollectionChangeWriter::collectionMounted(bool created)
{
    auto *write = new PendingWrite();
    if (created) {
        write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_Created);
    } else {
        write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_Opened);
    }
    write->change.set_uri(m_collection->getCollectionUrl().toString());
    enqueue(write);
}

void
CollectionChangeWriter::OnWriteDone(bool ok)
{
    TU_LOG_INFO_IF(!ok) << "write failed";
    absl::MutexLock locker(&m_lock);
    auto *done = m_head;
    m_head = m_head->next;
    delete done;
    if (m_head != nullptr) {
        StartWrite(&m_head->change);
    }
}

void
CollectionChangeWriter::OnDone()
{
    TU_LOG_INFO << "finished streaming collection changes";
    delete this;
}

void
CollectionChangeWriter::containerAdded(const tempo_utils::Url &containerUrl)
{
    auto *write = new PendingWrite();
    write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_AddedContainer);
    write->change.set_uri(containerUrl.toString());
    enqueue(write);
}

void
CollectionChangeWriter::containerRemoved(const tempo_utils::Url &containerUrl)
{
    auto *write = new PendingWrite();
    write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_RemovedContainer);
    write->change.set_uri(containerUrl.toString());
    enqueue(write);
}

void
CollectionChangeWriter::datasetAdded(const tempo_utils::Url &datasetUrl)
{
    auto *write = new PendingWrite();
    write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_AddedDataset);
    write->change.set_uri(datasetUrl.toString());
    enqueue(write);
}

void
CollectionChangeWriter::datasetRemoved(const tempo_utils::Url &datasetUrl)
{
    auto *write = new PendingWrite();
    write->change.set_type(groove_mount::CollectionChange_CollectionChangeType_RemovedDataset);
    write->change.set_uri(datasetUrl.toString());
    enqueue(write);
}

void
CollectionChangeWriter::enqueue(PendingWrite *write)
{
    TU_ASSERT (write != nullptr);

    absl::MutexLock locker(&m_lock);
    if (m_head == nullptr) {
        m_head = write;
        m_tail = write;
        StartWrite(&m_head->change);
    } else {
        m_tail->next = write;
        m_tail = write;
    }
}
