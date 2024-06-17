
#include <groove_agent/sync_service.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_storage/groove_shard.h>
#include <groove_storage/shard_state.h>
#include <groove_storage/storage_types.h>
#include <groove_sync/sync_service.grpc.pb.h>
#include <groove_sync/sync_service.pb.h>
#include <tempo_utils/memory_bytes.h>

SyncService::SyncService(
    const tempo_utils::Url &listenerUrl,
    StorageSupervisor *supervisor,
    std::string_view agentName)
    : m_listenerUrl(listenerUrl),
      m_supervisor(supervisor),
      m_agentName(agentName)
{
    TU_ASSERT (m_listenerUrl.isValid());
    TU_ASSERT (m_supervisor != nullptr);
}

grpc::ServerUnaryReactor *
SyncService::DescribeDataset(
    grpc::CallbackServerContext *context,
    const groove_sync::DescribeDatasetRequest *request,
    groove_sync::DescribeDatasetResult *response)
{
    TU_LOG_INFO << "client calls DescribeDataset";

    auto *reactor = context->DefaultReactor();

    //
    if (request->dataset_uri().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing dataset uri"));
        return reactor;
    }
    auto datasetUrl = tempo_utils::Url::fromString(request->dataset_uri());
    if (!datasetUrl.isValid()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "dataset url is invalid"));
        return reactor;
    }
    auto dataset = m_supervisor->getDataset(datasetUrl);
    if (dataset == nullptr) {
        reactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, "dataset not found"));
        return reactor;
    }

    auto schema = dataset->getSchema();
    auto schemaBytes = schema.bytesView();

    response->set_schema_bytes(std::string((const char *) schemaBytes.data(), schemaBytes.size()));
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

grpc::ServerWriteReactor<groove_sync::DatasetChange> *
SyncService::StreamDatasetChanges(
    grpc::CallbackServerContext *context,
    const groove_sync::StreamDatasetChangesRequest *request)
{
    TU_LOG_INFO << "client calls StreamDatasetChanges";

    //
    if (request->dataset_uri().empty()) {
        auto *stream = new DatasetChangeStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing dataset uri"));
        return stream;
    }
    auto datasetUrl = tempo_utils::Url::fromString(request->dataset_uri());
    if (!datasetUrl.isValid()) {
        auto *stream = new DatasetChangeStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "dataset url is invalid"));
        return stream;
    }
    auto dataset = m_supervisor->getDataset(datasetUrl);
    if (dataset == nullptr) {
        auto *stream = new DatasetChangeStream();
        stream->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, "dataset not found"));
        return stream;
    }

    auto *stream = new DatasetChangeStream(datasetUrl, dataset);
    return stream;
}

grpc::ServerUnaryReactor *
SyncService::GetShards(
    grpc::CallbackServerContext *context,
    const groove_sync::GetShardsRequest *request,
    groove_sync::GetShardsResult *response)
{
    TU_LOG_INFO << "client calls GetShards";

    auto *reactor = context->DefaultReactor();

    //
    if (request->dataset_uri().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing dataset uri"));
        return reactor;
    }
    auto datasetUrl = tempo_utils::Url::fromString(request->dataset_uri());
    if (!datasetUrl.isValid()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "dataset url is invalid"));
        return reactor;
    }
    if (request->model_id().empty()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing model id"));
        return reactor;
    }
    auto modelId = request->model_id();

    //
    auto dataset = m_supervisor->getDataset(datasetUrl);
    if (dataset == nullptr) {
        reactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, "dataset not found"));
        return reactor;
    }
    if (!dataset->hasModel(modelId)) {
        reactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, "model not found"));
        return reactor;
    }

    TU_LOG_INFO << "requesting shards for model " << modelId << " in dataset " << datasetUrl;

    //
    groove_storage::ShardState state;
    state.setDatasetUrl(datasetUrl);
    state.setModelId(modelId);
    state.setTimestamp(ToUnixSeconds(absl::Now()));
    switch (request->interval_case()) {
        case groove_sync::GetShardsRequest::kCat: {
            TU_LOG_INFO << "category interval requested";
            reactor->Finish(grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "category interval"));
            return reactor;
        }
        case groove_sync::GetShardsRequest::kDbl: {
            TU_LOG_INFO << "double interval requested";
            const auto &dbl = request->dbl();
            state.setDoubleInterval(dbl.start(), dbl.end());
            break;
        }
        case groove_sync::GetShardsRequest::kI64: {
            TU_LOG_INFO << "int64 interval requested";
            const auto &i64 = request->i64();
            state.setInt64Interval(i64.start(), i64.end());
            break;
        }
        default: {
            reactor->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid interval"));
            return reactor;
        }
    }
    auto toShardResult = state.toShard();
    if (toShardResult.isStatus()) {
        reactor->Finish(grpc::Status(grpc::StatusCode::INTERNAL,
            "failed to generate shard", std::string(toShardResult.getStatus().getMessage())));
        return reactor;
    }
    auto shard = toShardResult.getResult();
    TU_LOG_INFO << "serialized shard";

    auto shardBytes = shard.bytesView();

    auto *shard_ = response->add_shards();
    shard_->set_location(std::string());
    shard_->set_shard_bytes(std::string((const char *) shardBytes.data(), shardBytes.size()));
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

groove_data::DataKeyType interval_to_key_type(groove_storage::IntervalType interval)
{
    switch (interval) {
        case groove_storage::IntervalType::Category:
            return groove_data::DataKeyType::KEY_CATEGORY;
        case groove_storage::IntervalType::Double:
            return groove_data::DataKeyType::KEY_DOUBLE;
        case groove_storage::IntervalType::Int64:
            return groove_data::DataKeyType::KEY_INT64;
        default:
            return groove_data::DataKeyType::KEY_UNKNOWN;
    }
}

grpc::ServerWriteReactor<groove_sync::DataFrame> *
SyncService::GetData(
    grpc::CallbackServerContext *context,
    const groove_sync::GetDataRequest *request)
{
    TU_LOG_INFO << "client calls GetData";

    if (request->shard_bytes().empty()) {
        auto *stream = new DataFrameStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "missing shard"));
        return stream;
    }

    auto shardBytes = tempo_utils::MemoryBytes::copy(request->shard_bytes());
    if (!groove_storage::GrooveShard::verify(shardBytes->getSpan())) {
        auto *stream = new DataFrameStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid shard", "shard failed to verify"));
        return stream;
    }

    groove_storage::GrooveShard shard(shardBytes);
    TU_LOG_INFO << "shard dataset is " << shard.getDatasetUrl();
    TU_LOG_INFO << "shard model is " << shard.getModelId();
    TU_LOG_INFO << "shard timestamp is " << shard.getTimestamp();

    auto datasetUrl = shard.getDatasetUrl();
    if (!m_supervisor->hasDataset(datasetUrl)) {
        auto *stream = new DataFrameStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid shard", "no such dataset"));
        return stream;
    }
    auto dataset = m_supervisor->getDataset(datasetUrl);

    auto modelId = shard.getModelId();
    if (!dataset->hasModel(modelId)) {
        auto *stream = new DataFrameStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid shard", "no such model"));
        return stream;
    }
    auto model = dataset->getModel(modelId);

    if (interval_to_key_type(shard.getIntervalType()) != model->getKeyType()) {
        auto *stream = new DataFrameStream();
        stream->Finish(grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "invalid shard"));
        return stream;
    }

    auto *stream = new DataFrameStream(shard, model);
    return stream;
}

DatasetChangeStream::DatasetChangeStream()
{
}

DatasetChangeStream::DatasetChangeStream(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<groove_model::AbstractDataset> dataset)
    : m_datasetUrl(datasetUrl),
      m_dataset(dataset)
{
    auto schema = m_dataset->getSchema().getSchema();

    // special case: if dataset has no models, then finish immediately
    if (schema.numModels() == 0) {
        Finish(grpc::Status::OK);
        return;
    }

    // otherwise create a DatasetChange for each model in the dataset
    for (int i = 0; i < schema.numModels(); i++) {
        const auto model = schema.getModel(i);

        auto *pending = new PendingWrite();
        pending->next = nullptr;
        pending->change.set_type(groove_sync::DatasetChange::Arrived);
        pending->change.set_dataset_uri(m_datasetUrl.toString());
        pending->change.set_model_id(model.getModelId());

        switch (model.getKeyType()) {
            case groove_model::ModelKeyType::Category: {
                auto *cat = pending->change.mutable_cat();
                cat->add_start(std::string());
                cat->add_end(std::string());
                break;
            }
            case groove_model::ModelKeyType::Double: {
                auto *dbl = pending->change.mutable_dbl();
                dbl->set_start(std::numeric_limits<double>::min());
                dbl->set_end(std::numeric_limits<double>::max());
                break;
            }
            case groove_model::ModelKeyType::Int64: {
                auto *i64 = pending->change.mutable_i64();
                i64->set_start(std::numeric_limits<tu_int64>::min());
                i64->set_end(std::numeric_limits<tu_int64>::max());
                break;
            }
            default:
                break;
        }

        if (m_head == nullptr) {
            m_head = pending;
            m_tail = pending;
        } else {
            m_tail->next = pending;
        }
    }

    StartWrite(&m_head->change);
}

DatasetChangeStream::~DatasetChangeStream()
{
    while (m_head != nullptr) {
        auto *next = m_head->next;
        delete m_head;
        m_head = next;
    }
}

void
DatasetChangeStream::OnWriteDone(bool ok)
{
    if (!ok) {
        TU_LOG_ERROR << "write failed";
        return;
    }

    auto *next = m_head->next;
    delete m_head;
    m_head = next;
    if (m_head != nullptr) {
        StartWrite(&m_head->change);
    } else {
        Finish(grpc::Status::OK);
    }
}

void
DatasetChangeStream::OnDone()
{
    TU_LOG_INFO << "finished streaming dataset changes";
    delete this;
}

DataFrameStream::DataFrameStream()
{
}

DataFrameStream::DataFrameStream(groove_storage::GrooveShard shard, std::shared_ptr<groove_model::GrooveModel> model)
    : m_shard(shard),
      m_model(model)
{
    TU_ASSERT (m_shard.isValid());
    TU_ASSERT (m_model != nullptr);

    TU_LOG_INFO << "streaming data for model "<< *m_model->getModelId() << " of dataset " << m_model->getDatasetUrl();

    m_currColumn = model->columnsBegin();
    if (m_currColumn != m_model->columnsEnd()) {
        startNextFrame();
    } else {
        Finish(grpc::Status::OK);
    }
}

DataFrameStream::~DataFrameStream()
{
}

void
DataFrameStream::OnWriteDone(bool ok)
{
    if (!ok) {
        TU_LOG_ERROR << "write failed";
        return;
    }

    startNextFrame();
}

void
DataFrameStream::OnDone()
{
    TU_LOG_INFO << "finished streaming data";
    delete this;
}

groove_data::CategoryRange category_interval_to_range(const groove_storage::GrooveShard &shard)
{
    TU_ASSERT (shard.getIntervalType() == groove_storage::IntervalType::Category);

    groove_data::CategoryRange range;
    range.start = Option<groove_data::Category>(shard.getCategoryIntervalStart());
    range.start_exclusive = false;
    range.end = Option<groove_data::Category>(shard.getCategoryIntervalEnd());
    range.end_exclusive = false;

    return range;
}

groove_data::DoubleRange double_interval_to_range(const groove_storage::GrooveShard &shard)
{
    TU_ASSERT (shard.getIntervalType() == groove_storage::IntervalType::Double);
    groove_data::DoubleRange range;
    range.start = Option<double>(shard.getDoubleIntervalStart());
    range.start_exclusive = false;
    range.end = Option<double>(shard.getDoubleIntervalEnd());
    range.end_exclusive = false;
    return range;
}

groove_data::Int64Range int64_interval_to_range(const groove_storage::GrooveShard &shard)
{
    TU_ASSERT (shard.getIntervalType() == groove_storage::IntervalType::Int64);
    groove_data::Int64Range range;
    range.start = Option<tu_int64>(shard.getInt64IntervalStart());
    range.start_exclusive = false;
    range.end = Option<tu_int64>(shard.getInt64IntervalEnd());
    range.end_exclusive = false;
    return range;
}

template <typename DefType, typename RangeType>
static tempo_utils::Result<std::forward_list<std::shared_ptr<groove_data::BaseFrame>>>
get_indexed_column_frames(
    std::shared_ptr<groove_model::GrooveModel> model,
    const std::string &columnId,
    const RangeType &range)
{
    auto getIndexedColumnResult = model->getIndexedColumn<DefType>(columnId);
    if (getIndexedColumnResult.isStatus())
        return getIndexedColumnResult.getStatus();
    auto column = getIndexedColumnResult.getResult();

    auto getFramesResult = column->getFrames(range);
    if (getFramesResult.isStatus())
        return getFramesResult.getStatus();

    // FIXME: this copying is yucky
    std::forward_list<std::shared_ptr<groove_data::BaseFrame>> frames;
    auto tail = frames.before_begin();
    for (const auto &frame : getFramesResult.getResult()) {
        tail = frames.insert_after(tail, frame);
    }
    return frames;
}

static tempo_utils::Result<std::forward_list<std::shared_ptr<groove_data::BaseFrame>>>
get_indexed_category_column_frames(
    std::shared_ptr<groove_model::GrooveModel> model,
    const std::string &columnId,
    const groove_model::ColumnDef &def,
    const groove_data::CategoryRange &range)
{
    TU_ASSERT (def.getCollation() == groove_data::CollationMode::COLLATION_INDEXED);
    TU_ASSERT (def.getKey() == groove_data::DataKeyType::KEY_CATEGORY);

    switch (def.getValue()) {
        case groove_data::DataValueType::VALUE_TYPE_DOUBLE:
            return get_indexed_column_frames<groove_model::CategoryDouble,groove_data::CategoryRange>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_INT64:
            return get_indexed_column_frames<groove_model::CategoryInt64,groove_data::CategoryRange>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_STRING:
            return get_indexed_column_frames<groove_model::CategoryString,groove_data::CategoryRange>(
                model, columnId, range);
        default:
            return groove_model::ModelStatus::forCondition(
                groove_model::ModelCondition::kModelInvariant, "invalid column type");
    }
}

static tempo_utils::Result<std::forward_list<std::shared_ptr<groove_data::BaseFrame>>>
get_indexed_double_column_frames(
    std::shared_ptr<groove_model::GrooveModel> model,
    const std::string &columnId,
    const groove_model::ColumnDef &def,
    const groove_data::DoubleRange &range)
{
    TU_ASSERT (def.getCollation() == groove_data::CollationMode::COLLATION_INDEXED);
    TU_ASSERT (def.getKey() == groove_data::DataKeyType::KEY_DOUBLE);

    switch (def.getValue()) {
        case groove_data::DataValueType::VALUE_TYPE_DOUBLE:
            return get_indexed_column_frames<groove_model::DoubleDouble,groove_data::DoubleRange>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_INT64:
            return get_indexed_column_frames<groove_model::DoubleInt64,groove_data::DoubleRange>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_STRING:
            return get_indexed_column_frames<groove_model::DoubleString,groove_data::DoubleRange>(
                model, columnId, range);
        default:
            return groove_model::ModelStatus::forCondition(
                groove_model::ModelCondition::kModelInvariant, "invalid column type");
    }
}

static tempo_utils::Result<std::forward_list<std::shared_ptr<groove_data::BaseFrame>>>
get_indexed_int64_column_frames(
    std::shared_ptr<groove_model::GrooveModel> model,
    const std::string &columnId,
    const groove_model::ColumnDef &def,
    const groove_data::Int64Range &range)
{
    TU_ASSERT (def.getCollation() == groove_data::CollationMode::COLLATION_INDEXED);
    TU_ASSERT (def.getKey() == groove_data::DataKeyType::KEY_INT64);

    switch (def.getValue()) {
        case groove_data::DataValueType::VALUE_TYPE_DOUBLE:
            return get_indexed_column_frames<groove_model::Int64Double,groove_data::Int64Range>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_INT64:
            return get_indexed_column_frames<groove_model::Int64Int64,groove_data::Int64Range>(
                model, columnId, range);
        case groove_data::DataValueType::VALUE_TYPE_STRING:
            return get_indexed_column_frames<groove_model::Int64String,groove_data::Int64Range>(
                model, columnId, range);
        default:
            return groove_model::ModelStatus::forCondition(
                groove_model::ModelCondition::kModelInvariant, "invalid column type");
    }
}

bool
DataFrameStream::startNextColumn()
{
    // if there are no more columns then we are done
    if (m_currColumn == m_model->columnsEnd()) {
        TU_LOG_INFO << "reached the columns end";
        Finish(grpc::Status::OK);
        return false;
    }

    TU_LOG_INFO << "starting next column";

    const auto &columnId = m_currColumn->first;
    const auto &columnDef = m_currColumn->second;

    switch (m_shard.getIntervalType()) {
        case groove_storage::IntervalType::Category: {
            auto range = category_interval_to_range(m_shard);
            TU_LOG_INFO << "reading column " << columnId << " using " << range;
            auto getFramesResult = get_indexed_category_column_frames(m_model, columnId, columnDef, range);
            if (getFramesResult.isStatus()) {
                Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to read column"));
                return false;
            }
            m_frames = getFramesResult.getResult();
            break;
        }
        case groove_storage::IntervalType::Double: {
            auto range = double_interval_to_range(m_shard);
            TU_LOG_INFO << "reading column " << columnId << " using " << range;
            auto getFramesResult = get_indexed_double_column_frames(m_model, columnId, columnDef, range);
            if (getFramesResult.isStatus()) {
                Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to read column"));
                return false;
            }
            m_frames = getFramesResult.getResult();
            break;
        }
        case groove_storage::IntervalType::Int64: {
            auto range = int64_interval_to_range(m_shard);
            TU_LOG_INFO << "reading column " << columnId << " using " << range;
            auto getFramesResult = get_indexed_int64_column_frames(m_model, columnId, columnDef, range);
            if (getFramesResult.isStatus()) {
                Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to read column"));
                return false;
            }
            m_frames = getFramesResult.getResult();
            break;
        }
        default:
            Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to read column", "invalid interval"));
            return false;
    }

    int count = 0;
    for (auto iterator = m_frames.cbegin(); iterator != m_frames.cend(); iterator++) {
        count++;
    }
    TU_LOG_INFO << "loaded " << count << " frames from column " << columnId << " of model " << *m_model->getModelId();

    m_currColumn++; // advance the column iterator
    return true;
}

std::shared_ptr<const arrow::Buffer> make_buffer_from_frame(std::shared_ptr<groove_data::BaseFrame> frame)
{
    auto table = frame->getUnderlyingTable();
    auto schema = table->schema();

    auto createBufResult = arrow::io::BufferOutputStream::Create();
    if (!createBufResult.ok())
        return {};
    auto stream = *createBufResult;

    auto makeWriterResult = arrow::ipc::MakeStreamWriter(stream, schema);
    if (!makeWriterResult.ok())
        return {};
    auto writer = *makeWriterResult;

    arrow::Status status;
    status = writer->WriteTable(*table);
    if (!status.ok())
        return {};
    status = writer->Close();
    if (!status.ok())
        return {};
    auto finishStreamResult = stream->Finish();
    if (!finishStreamResult.ok())
        return {};
    return *finishStreamResult;
}

bool
DataFrameStream::startNextFrame()
{
    TU_LOG_INFO << "starting next frame";

    // frames list starts empty. we call startNextColumn until either 1) frames list
    // becomes non-empty, meaning we have loaded a column with data in the specified range,
    // or 2) startNextColumn returns false indicating either there are no more columns to
    // load or we encountered an error which requires the stream to terminate.
    while (m_frames.empty()) {
        if (!startNextColumn()) {
            TU_LOG_INFO << "no more columns to load for model " << *m_model->getModelId();
            return false;
        }
    }

    // take the frame from the head of the list
    auto frame = m_frames.front();
    m_frames.pop_front();
    TU_LOG_INFO << "preparing outgoing frame with " << frame->numVectors() << " vectors and " << frame->getSize() << " rows";

    // use the key index from the frame
    m_outgoing.set_key_index(frame->getKeyFieldIndex());

    // add value field for each vector in the frame
    for (auto iterator = frame->vectorsBegin(); iterator != frame->vectorsEnd(); iterator++) {
        const auto vector = iterator->second;
        auto *valueField = m_outgoing.add_value_fields();
        valueField->set_val_index(vector->getValFieldIndex());
        valueField->set_fid_index(vector->getFidFieldIndex());
    }

    // serialize the frame
    m_buffer = make_buffer_from_frame(frame);
    if (m_buffer == nullptr) {
        Finish(grpc::Status(grpc::StatusCode::INTERNAL, "failed to create buffer"));
        return false;
    }

    // NOTE: we hold buffer reference for lifetime of this string so its ok to not copy
    std::string frameBytes((const char *) m_buffer->data(), m_buffer->size());

    switch (frame->getFrameType()) {
        case groove_data::DataFrameType::FRAME_TYPE_CATEGORY:
            m_outgoing.set_cat(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_DOUBLE:
            m_outgoing.set_dbl(std::move(frameBytes));
            break;
        case groove_data::DataFrameType::FRAME_TYPE_INT64:
            m_outgoing.set_i64(std::move(frameBytes));
            break;
        default:
            TU_UNREACHABLE();
    }

    StartWrite(&m_outgoing);
    return true;
}