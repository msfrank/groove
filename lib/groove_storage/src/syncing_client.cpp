
#include <grpcpp/create_channel.h>
#include <groove_model/shared_string_buffer.h>
#include <groove_model/base_page.h>
#include <groove_data/table_utils.h>
#include <groove_model/groove_schema.h>
#include <groove_storage/syncing_client.h>
#include <tempo_utils/memory_bytes.h>

groove_storage::SyncingClient::SyncingClient(
    const tempo_utils::Url &endpointUrl,
    std::shared_ptr<grpc::ChannelCredentials> credentials,
    const std::string &endpointServerName)
    : m_endpointUrl(endpointUrl),
      m_credentials(credentials),
      m_endpointServerName(endpointServerName)
{
    TU_ASSERT (m_endpointUrl.isValid());
    TU_ASSERT (m_credentials != nullptr);
}

tempo_utils::Status
groove_storage::SyncingClient::connect()
{
    absl::MutexLock lock(&m_lock);

    if (m_stub)
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "syncing client is already connected");

    // construct the client
    grpc::ChannelArguments channelArguments;
    if (!m_endpointServerName.empty())
        channelArguments.SetSslTargetNameOverride(m_endpointServerName);
    m_channel = grpc::CreateCustomChannel(m_endpointUrl.toString(), m_credentials, channelArguments);
    m_stub = groove_sync::SyncService::NewStub(m_channel);

    return StorageStatus::ok();
}

tempo_utils::Status
groove_storage::SyncingClient::shutdown()
{
    absl::MutexLock lock(&m_lock);
    if (m_stub) {
        TU_LOG_INFO << "shutting down communication with " << m_endpointUrl;
        m_stub.reset();
    }
    return StorageStatus::ok();
}

bool
groove_storage::SyncingClient::addDatasetWatch(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<DatasetChangeHandler> watch,
    const std::string &key)
{
    std::shared_ptr<DatasetWatchPriv> priv;
    {
        absl::MutexLock lock(&m_lock);

        //
        if (!m_datasets.contains(datasetUrl)) {
            priv = std::make_shared<DatasetWatchPriv>();
            priv->handlers[key] = watch;
            priv->client = this;
            priv->stream = new DatasetChangeStream(m_stub.get(), datasetUrl, priv);
            return true;
        }
        priv = m_datasets.at(datasetUrl);
    }

    absl::MutexLock lock(&priv->lock);
    if (priv->handlers.contains(key))
        return false;
    priv->handlers[key] = watch;
    return true;
}

bool
groove_storage::SyncingClient::removeDatasetWatch(
    const tempo_utils::Url &datasetUrl,
    const std::string &key)
{
    std::shared_ptr<DatasetWatchPriv> priv;
    {
        absl::MutexLock lock(&m_lock);
        if (!m_datasets.contains(datasetUrl))
            return false;
        priv = m_datasets.at(datasetUrl);
    }

    absl::MutexLock lock(&priv->lock);
    if (!priv->handlers.contains(key))
        return false;
    priv->handlers.erase(key);
    return true;
}

bool
groove_storage::SyncingClient::describeDataset(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<DescribeDatasetHandler> handler)
{
    new DescribeDatasetReactor(m_stub.get(), datasetUrl, handler);
    return true;
}

bool
groove_storage::SyncingClient::getShards(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::CategoryRange &range,
    std::shared_ptr<GetShardsHandler> handler)
{
    new GetShardsReactor(m_stub.get(), datasetUrl, modelId, range, handler);
    return true;
}

bool
groove_storage::SyncingClient::getShards(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::DoubleRange &range,
    std::shared_ptr<GetShardsHandler> handler)
{
    new GetShardsReactor(m_stub.get(), datasetUrl, modelId, range, handler);
    return true;
}

bool
groove_storage::SyncingClient::getShards(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::Int64Range &range,
    std::shared_ptr<GetShardsHandler> handler)
{
    new GetShardsReactor(m_stub.get(), datasetUrl, modelId, range, handler);
    return true;
}

bool
groove_storage::SyncingClient::getData(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::string &shardBytes,
    std::shared_ptr<GetDataHandler> handler)
{
    new DataFrameStream(m_stub.get(), datasetUrl, modelId, shardBytes, handler);
    return true;
}

tempo_utils::Result<std::shared_ptr<groove_storage::SyncingClient>>
groove_storage::SyncingClient::create(
    const tempo_utils::Url &endpointUrl,
    std::shared_ptr<grpc::ChannelCredentials> credentials,
    const std::string &endpointServerName)
{
    return std::shared_ptr<SyncingClient>(
        new SyncingClient(endpointUrl, credentials, endpointServerName));
}

inline groove_data::CategoryRange
make_category_range(const groove_sync::CategoryInterval *interval)
{
    std::vector<std::string> start;
    for (int i = 0; i < interval->start_size(); i++) {
        start.push_back(interval->start(i));
    }
    std::vector<std::string> end;
    for (int i = 0; i < interval->end_size(); i++) {
        start.push_back(interval->end(i));
    }
    groove_data::CategoryRange range;
    range.start = Option<groove_data::Category>(groove_data::Category(start));
    range.end = Option<groove_data::Category>(groove_data::Category(end));
    range.start_exclusive = false;
    range.end_exclusive = false;
    return range;
}

inline groove_data::DoubleRange
make_double_range(const groove_sync::DoubleInterval *interval)
{
    groove_data::DoubleRange range;
    range.start = Option<double>(interval->start());
    range.end = Option<double>(interval->end());
    range.start_exclusive = false;
    range.end_exclusive = false;
    return range;
}

inline groove_data::Int64Range
make_int64_range(const groove_sync::Int64Interval *interval)
{
    groove_data::Int64Range range;
    range.start = Option<tu_int64>(interval->start());
    range.end = Option<tu_int64>(interval->end());
    range.start_exclusive = false;
    range.end_exclusive = false;
    return range;
}

void
groove_storage::SyncingClient::DatasetWatchPriv::notify(const groove_sync::DatasetChange *change)
{
    auto changeType = change->type();
    auto datasetUrl = tempo_utils::Url::fromString(change->dataset_uri());
    auto modelId = change->model_id();

    switch (change->interval_case()) {
        case groove_sync::DatasetChange::IntervalCase::kCat: {
            auto range = make_category_range(&change->cat());
            absl::MutexLock lock_(&lock);
            for (auto iterator = handlers.cbegin(); iterator != handlers.cend(); iterator++) {
                auto watch = iterator->second;
                watch->categoryModelChanged(changeType, datasetUrl, modelId, range);
            }
            break;
        }
        case groove_sync::DatasetChange::IntervalCase::kDbl: {
            auto range = make_double_range(&change->dbl());
            absl::MutexLock lock_(&lock);
            for (auto iterator = handlers.cbegin(); iterator != handlers.cend(); iterator++) {
                auto watch = iterator->second;
                watch->doubleModelChanged(changeType, datasetUrl, modelId, range);
            }
            break;
        }
        case groove_sync::DatasetChange::IntervalCase::kI64: {
            auto range = make_int64_range(&change->i64());
            absl::MutexLock lock_(&lock);
            for (auto iterator = handlers.cbegin(); iterator != handlers.cend(); iterator++) {
                auto watch = iterator->second;
                watch->int64ModelChanged(changeType, datasetUrl, modelId, range);
            }
            break;
        }
        default:
            break;
    }
}

groove_storage::DatasetChangeStream::DatasetChangeStream(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<AbstractDatasetWatcher> watcher)
    : m_datasetUrl(datasetUrl),
      m_watcher(watcher)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_watcher != nullptr);

    m_context.AddMetadata("x-zuri-dataset-url", datasetUrl.toString());
    m_request.set_dataset_uri(datasetUrl.toString());
    stub->async()->StreamDatasetChanges(&m_context, &m_request, this);
    StartCall();
}

groove_storage::DatasetChangeStream::~DatasetChangeStream()
{
}

void
groove_storage::DatasetChangeStream::OnReadInitialMetadataDone(bool ok)
{
    TU_LOG_ERROR_IF (!ok) << "failed to read initial metadata";
    StartRead(&m_incoming);
}

void
groove_storage::DatasetChangeStream::OnReadDone(bool ok)
{
    if (ok) {
        TU_LOG_INFO << "received change for dataset " << m_datasetUrl;
        m_watcher->notify(&m_incoming);
        m_incoming.Clear();
        StartRead(&m_incoming);
    } else {
        TU_LOG_ERROR << "read failed";
    }
}

void
groove_storage::DatasetChangeStream::OnDone(const grpc::Status &status)
{
    TU_LOG_INFO << "remote end closed with status " << status.error_message() << " (" << status.error_details() << ")";
    delete this;
}

groove_storage::DescribeDatasetReactor::DescribeDatasetReactor(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<DescribeDatasetHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(datasetUrl.toString());
    stub->async()->DescribeDataset(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::DescribeDatasetReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        std::shared_ptr<const tempo_utils::MemoryBytes> schemaBytes = tempo_utils::MemoryBytes::copy(m_result.schema_bytes());
        if (!groove_model::GrooveSchema::verify(schemaBytes->getSpan())) {
            m_handler->describeDatasetFinished(m_datasetUrl, {},
                StorageStatus::forCondition(StorageCondition::kStorageInvariant, "invalid schema"));
        } else {
            groove_model::GrooveSchema schema(std::static_pointer_cast<const tempo_utils::ImmutableBytes>(schemaBytes));
            m_handler->describeDatasetFinished(m_datasetUrl, schema, StorageStatus::ok());
        }
    } else {
        m_handler->describeDatasetFinished(m_datasetUrl, {},
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::GetShardsReactor::GetShardsReactor(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::CategoryRange &range,
    std::shared_ptr<GetShardsHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(datasetUrl.toString());
    m_request.set_model_id(modelId);
    auto *interval = m_request.mutable_cat();
    if (!range.start.isEmpty()) {
        const auto category = range.start.getValue();
        for (auto iterator = category.cbegin(); iterator != category.cend(); iterator++) {
            interval->add_start(**iterator);
        }
    }
    if (!range.end.isEmpty()) {
        const auto category = range.end.getValue();
        for (auto iterator = category.cbegin(); iterator != category.cend(); iterator++) {
            interval->add_end(**iterator);
        }
    }
    stub->async()->GetShards(&m_context, &m_request, &m_result, this);
    StartCall();
}

groove_storage::GetShardsReactor::GetShardsReactor(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::DoubleRange &range,
    std::shared_ptr<GetShardsHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(datasetUrl.toString());
    m_request.set_model_id(modelId);
    auto *interval = m_request.mutable_dbl();
    if (!range.start.isEmpty()) {
        interval->set_start(range.start.getValue());
    }
    if (!range.end.isEmpty()) {
        interval->set_end(range.end.getValue());
    }
    stub->async()->GetShards(&m_context, &m_request, &m_result, this);
    StartCall();
}

groove_storage::GetShardsReactor::GetShardsReactor(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::Int64Range &range,
    std::shared_ptr<GetShardsHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_handler != nullptr);

    m_request.set_dataset_uri(datasetUrl.toString());
    m_request.set_model_id(modelId);
    auto *interval = m_request.mutable_i64();
    if (!range.start.isEmpty()) {
        interval->set_start(range.start.getValue());
    }
    if (!range.end.isEmpty()) {
        interval->set_end(range.end.getValue());
    }
    stub->async()->GetShards(&m_context, &m_request, &m_result, this);
    StartCall();
}

void
groove_storage::GetShardsReactor::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        std::vector<std::string> shards;
        for (const auto &shard : m_result.shards()) {
            shards.push_back(shard.shard_bytes());
        }
        m_handler->getShardsFinished(m_datasetUrl, m_modelId, shards, StorageStatus::ok());
    } else {
        m_handler->getShardsFinished(m_datasetUrl, m_modelId, {},
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

groove_storage::DataFrameStream::DataFrameStream(
    groove_sync::SyncService::Stub *stub,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::string &shardBytes,
    std::shared_ptr<GetDataHandler> handler)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_handler(handler)
{
    TU_ASSERT (stub != nullptr);
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_handler != nullptr);

    m_context.AddMetadata("x-zuri-dataset-url", datasetUrl.toString());
    m_context.AddMetadata("x-zuri-model-id", modelId);
    m_request.set_shard_bytes(shardBytes);
    stub->async()->GetData(&m_context, &m_request, this);
    StartCall();
}

groove_storage::DataFrameStream::~DataFrameStream()
{
}

void
groove_storage::DataFrameStream::OnReadInitialMetadataDone(bool ok)
{
    TU_LOG_ERROR_IF (!ok) << "failed to read initial metadata";
    StartRead(&m_incoming);
}

void
groove_storage::DataFrameStream::OnReadDone(bool ok)
{
    // if read was not ok, then ignore the rpc and return immediately
    if (!ok) {
        return;
    }

    std::vector<std::pair<int,int>> valueFields;
    for (const auto &valueField : m_incoming.value_fields()) {
        valueFields.push_back(std::pair<int,int>(valueField.val_index(), valueField.fid_index()));
    }
    auto keyFieldIndex = m_incoming.key_index();

    switch (m_incoming.frame_case()) {
        case groove_sync::DataFrame::kCat: {
            auto buffer = groove_model::SharedStringBuffer::create(m_incoming.cat());
            auto makeTableResult = groove_data::make_table(buffer);
            if (makeTableResult.isStatus())
                break;
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::CategoryFrame::create(table, keyFieldIndex, valueFields);
            if (createFrameResult.isStatus())
                break;
            auto frame = createFrameResult.getResult();
            m_handler->categoryFrameReceived(m_datasetUrl, m_modelId, frame);
            break;
        }
        case groove_sync::DataFrame::kDbl: {
            auto buffer = groove_model::SharedStringBuffer::create(m_incoming.dbl());
            auto makeTableResult = groove_data::make_table(buffer);
            if (makeTableResult.isStatus())
                break;
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::DoubleFrame::create(table, keyFieldIndex, valueFields);
            if (createFrameResult.isStatus())
                break;
            auto frame = createFrameResult.getResult();
            m_handler->doubleFrameReceived(m_datasetUrl, m_modelId, frame);
            break;
        }
        case groove_sync::DataFrame::kI64: {
            auto buffer = groove_model::SharedStringBuffer::create(m_incoming.i64());
            auto makeTableResult = groove_data::make_table(buffer);
            if (makeTableResult.isStatus())
                break;
            auto table = makeTableResult.getResult();
            auto createFrameResult = groove_data::Int64Frame::create(table, keyFieldIndex, valueFields);
            if (createFrameResult.isStatus())
                break;
            auto frame = createFrameResult.getResult();
            m_handler->int64FrameReceived(m_datasetUrl, m_modelId, frame);
            break;
        }
        default:
            break;
    }

    m_incoming.Clear();
    StartRead(&m_incoming);
}

void
groove_storage::DataFrameStream::OnDone(const grpc::Status &status)
{
    if (status.ok()) {
        m_handler->getDataFinished(m_datasetUrl, m_modelId, StorageStatus::ok());
    } else {
        TU_LOG_ERROR_IF(!status.ok())
            << "remote end closed with status " << status.error_message()
            << " (" << status.error_details() << ")";
        m_handler->getDataFinished(m_datasetUrl, m_modelId,
            StorageStatus::forCondition(StorageCondition::kStorageInvariant, status.error_message()));
    }
    delete this;
}

void
groove_storage::DatasetChangeHandler::categoryModelChanged(
    groove_sync::DatasetChange::DatasetChangeType changeType,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::CategoryRange &range)
{
}

void
groove_storage::DatasetChangeHandler::doubleModelChanged(
    groove_sync::DatasetChange::DatasetChangeType changeType,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::DoubleRange &range)
{
}

void
groove_storage::DatasetChangeHandler::int64ModelChanged(
    groove_sync::DatasetChange::DatasetChangeType changeType,
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const groove_data::Int64Range &range)
{
}

void
groove_storage::DatasetChangeHandler::datasetChangesFinished(
    const tempo_utils::Url &datasetUrl,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::DescribeDatasetHandler::describeDatasetFinished(
    const tempo_utils::Url &datasetUrl,
    const groove_model::GrooveSchema &schema,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::GetShardsHandler::getShardsFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::vector<std::string> &shards,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}

void
groove_storage::GetDataHandler::categoryFrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::CategoryFrame> frame)
{
}

void
groove_storage::GetDataHandler::doubleFrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::DoubleFrame> frame)
{
}

void
groove_storage::GetDataHandler::int64FrameReceived(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::Int64Frame> frame)
{
}

void
groove_storage::GetDataHandler::getDataFinished(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const tempo_utils::Status &status)
{
    TU_LOG_WARN_IF(status.notOk()) << status;
}