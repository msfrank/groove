#ifndef GROOVE_AGENT_SYNC_SERVICE_H
#define GROOVE_AGENT_SYNC_SERVICE_H

#include <groove_io/dataset_reader.h>
#include <groove_storage/groove_shard.h>
#include <groove_sync/sync_service.grpc.pb.h>

#include "storage_supervisor.h"

class SyncService : public groove_sync::SyncService::CallbackService {

public:
    SyncService(
        const tempo_utils::Url &listenerUrl,
        StorageSupervisor *supervisor,
        std::string_view agentName);

    grpc::ServerUnaryReactor *
    DescribeDataset(
        grpc::CallbackServerContext *context,
        const groove_sync::DescribeDatasetRequest *request,
        groove_sync::DescribeDatasetResult *response) override;

    grpc::ServerWriteReactor<groove_sync::DatasetChange> *
    StreamDatasetChanges(
        grpc::CallbackServerContext *context,
        const groove_sync::StreamDatasetChangesRequest *request) override;

    grpc::ServerUnaryReactor *
    GetShards(
        grpc::CallbackServerContext *context,
        const groove_sync::GetShardsRequest *request,
        groove_sync::GetShardsResult *response) override;

    grpc::ServerWriteReactor<groove_sync::DataFrame> *
    GetData(
        grpc::CallbackServerContext *context,
        const groove_sync::GetDataRequest *request) override;

private:
    tempo_utils::Url m_listenerUrl;
    StorageSupervisor *m_supervisor;
    std::string m_agentName;
};

class DatasetChangeStream : public grpc::ServerWriteReactor<groove_sync::DatasetChange>
{
public:
    DatasetChangeStream();
    DatasetChangeStream(const tempo_utils::Url &datasetUrl, std::shared_ptr<groove_model::AbstractDataset> dataset);
    ~DatasetChangeStream();

    void OnWriteDone(bool ok) override;
    void OnDone() override;

    struct PendingWrite {
        groove_sync::DatasetChange change;
        PendingWrite *next;
    };

private:
    tempo_utils::Url m_datasetUrl;
    std::shared_ptr<groove_model::AbstractDataset> m_dataset;
    PendingWrite *m_head;
    PendingWrite *m_tail;
};

class DataFrameStream : public grpc::ServerWriteReactor<groove_sync::DataFrame>
{
public:
    DataFrameStream();
    DataFrameStream(groove_storage::GrooveShard shard, std::shared_ptr<groove_model::GrooveModel> model);
    ~DataFrameStream();

    void OnWriteDone(bool ok) override;
    void OnDone() override;

private:
    groove_storage::GrooveShard m_shard;
    std::shared_ptr<groove_model::GrooveModel> m_model;
    absl::flat_hash_map<std::string,groove_model::ColumnDef>::const_iterator m_currColumn;
    std::forward_list<std::shared_ptr<groove_data::BaseFrame>> m_frames;
    std::shared_ptr<const arrow::Buffer> m_buffer;
    groove_sync::DataFrame m_outgoing;

    bool startNextColumn();
    bool startNextFrame();
};

#endif // GROOVE_AGENT_SYNC_SERVICE_H