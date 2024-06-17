#ifndef GROOVE_STORAGE_SYNCING_CLIENT_H
#define GROOVE_STORAGE_SYNCING_CLIENT_H

#include <grpcpp/security/credentials.h>

#include <groove_data/category_frame.h>
#include <groove_data/data_types.h>
#include <groove_data/double_frame.h>
#include <groove_data/int64_frame.h>
#include <groove_model/groove_schema.h>
#include <groove_sync/sync_service.grpc.pb.h>
#include <tempo_utils/url.h>

#include "storage_result.h"

namespace groove_storage {

    class AbstractDatasetWatcher {
    public:
        virtual ~AbstractDatasetWatcher() = default;
        virtual void notify(const groove_sync::DatasetChange *change) = 0;
    };

    class DatasetChangeHandler {
    public:
        virtual ~DatasetChangeHandler() = default;
        virtual void categoryModelChanged(
            groove_sync::DatasetChange::DatasetChangeType changeType,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::CategoryRange &range);
        virtual void doubleModelChanged(
            groove_sync::DatasetChange::DatasetChangeType changeType,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::DoubleRange &range);
        virtual void int64ModelChanged(
            groove_sync::DatasetChange::DatasetChangeType changeType,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::Int64Range &range);
        virtual void datasetChangesFinished(
            const tempo_utils::Url &datasetUrl,
            const tempo_utils::Status &status);
    };

    class DescribeDatasetHandler {
    public:
        virtual ~DescribeDatasetHandler() = default;
        virtual void describeDatasetFinished(
            const tempo_utils::Url &datasetUrl,
            const groove_model::GrooveSchema &schema,
            const tempo_utils::Status &status);
    };

    class GetShardsHandler {
    public:
        virtual ~GetShardsHandler() = default;
        virtual void getShardsFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::vector<std::string> &shards,
            const tempo_utils::Status &status);
    };

    class GetDataHandler {
    public:
        virtual ~GetDataHandler() = default;
        virtual void categoryFrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::CategoryFrame> frame);
        virtual void doubleFrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::DoubleFrame> frame);
        virtual void int64FrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::Int64Frame> frame);
        virtual void getDataFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const tempo_utils::Status &status);
    };

    class DatasetChangeStream : public grpc::ClientReadReactor<groove_sync::DatasetChange> {
    public:
        DatasetChangeStream(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<AbstractDatasetWatcher> watcher);
        ~DatasetChangeStream();

        void OnReadInitialMetadataDone(bool ok) override;
        void OnReadDone(bool ok) override;
        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<AbstractDatasetWatcher> m_watcher;
        groove_sync::StreamDatasetChangesRequest m_request;
        grpc::ClientContext m_context;
        groove_sync::DatasetChange m_incoming;
    };

    class DescribeDatasetReactor : public grpc::ClientUnaryReactor {
    public:
        DescribeDatasetReactor(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DescribeDatasetHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<DescribeDatasetHandler> m_handler;
        groove_sync::DescribeDatasetRequest m_request;
        grpc::ClientContext m_context;
        groove_sync::DescribeDatasetResult m_result;
    };

    class GetShardsReactor : public grpc::ClientUnaryReactor {
    public:
        GetShardsReactor(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::CategoryRange &range,
            std::shared_ptr<GetShardsHandler> handler);
        GetShardsReactor(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::DoubleRange &range,
            std::shared_ptr<GetShardsHandler> handler);
        GetShardsReactor(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::Int64Range &range,
            std::shared_ptr<GetShardsHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
        std::shared_ptr<GetShardsHandler> m_handler;
        groove_sync::GetShardsRequest m_request;
        grpc::ClientContext m_context;
        groove_sync::GetShardsResult m_result;
    };

    class DataFrameStream : public grpc::ClientReadReactor<groove_sync::DataFrame> {
    public:
        DataFrameStream(
            groove_sync::SyncService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::string &shardBytes,
            std::shared_ptr<GetDataHandler> handler);
        ~DataFrameStream();

        void OnReadInitialMetadataDone(bool ok) override;
        void OnReadDone(bool ok) override;
        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
        std::shared_ptr<GetDataHandler> m_handler;
        groove_sync::GetDataRequest m_request;
        grpc::ClientContext m_context;
        groove_sync::DataFrame m_incoming;
    };

    class SyncingClient : public std::enable_shared_from_this<SyncingClient> {

    public:
        tempo_utils::Status connect();
        tempo_utils::Status shutdown();

        bool addDatasetWatch(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DatasetChangeHandler> handler,
            const std::string &key);
        bool removeDatasetWatch(
            const tempo_utils::Url &datasetUrl,
            const std::string &key);

        bool describeDataset(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DescribeDatasetHandler> handler);
        bool getShards(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::CategoryRange &range,
            std::shared_ptr<GetShardsHandler> handler);
        bool getShards(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::DoubleRange &range,
            std::shared_ptr<GetShardsHandler> handler);
        bool getShards(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::Int64Range &range,
            std::shared_ptr<GetShardsHandler> handler);
        bool getData(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::string &shardBytes,
            std::shared_ptr<GetDataHandler> handler);

        static tempo_utils::Result<std::shared_ptr<SyncingClient>> create(
            const tempo_utils::Url &endpointUrl,
            std::shared_ptr<grpc::ChannelCredentials> credentials,
            const std::string &endpointServerName);

    private:
        tempo_utils::Url m_endpointUrl;
        std::shared_ptr<grpc::ChannelCredentials> m_credentials;
        std::string m_endpointServerName;
        std::shared_ptr<grpc::Channel> m_channel;
        std::unique_ptr<groove_sync::SyncService::Stub> m_stub;

        struct DatasetWatchPriv : public AbstractDatasetWatcher {
            SyncingClient *client;
            DatasetChangeStream *stream;
            absl::flat_hash_map<std::string,std::shared_ptr<DatasetChangeHandler>> handlers;
            absl::Mutex lock;
            void notify(const groove_sync::DatasetChange *change) override;
        };
        absl::flat_hash_map<tempo_utils::Url,std::shared_ptr<DatasetWatchPriv>> m_datasets;
        absl::Mutex m_lock;

        SyncingClient(
            const tempo_utils::Url &endpointUrl,
            std::shared_ptr<grpc::ChannelCredentials> credentials,
            const std::string &endpointServerName);
    };
}

#endif // GROOVE_STORAGE_SYNCING_CLIENT_H