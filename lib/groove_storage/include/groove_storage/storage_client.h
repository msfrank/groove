#ifndef GROOVE_STORAGE_STORAGE_CLIENT_H
#define GROOVE_STORAGE_STORAGE_CLIENT_H

#include <grpcpp/security/credentials.h>

#include <groove_data/base_frame.h>
#include <groove_model/groove_schema.h>
#include <groove_mount/mount_service.grpc.pb.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/url.h>

#include "storage_result.h"

namespace groove_storage {

    // forward declarations
    class StorageClient;

    enum class MountMode {
        Open,
        OpenOrCreate,
        CreateOnly,
    };

    struct StorageIdentity {
        std::string agentName;
        tu_uint64 uptimeMillis;
    };

    class CollectionUnmounter {
    public:
        CollectionUnmounter(std::shared_ptr<StorageClient> client, tu_uint64 lsn);
        tempo_utils::Status unmount();
    private:
        std::shared_ptr<StorageClient> m_client;
        tu_uint64 m_lsn;
    };

    class MountCollectionHandler {
    public:
        virtual ~MountCollectionHandler() = default;
        virtual void collectionCreated(const tempo_utils::Url &collectionUrl);
        virtual void collectionOpened(const tempo_utils::Url &collectionUrl);
        virtual void containerAdded(const tempo_utils::Url &containerUrl);
        virtual void containerRemoved(const tempo_utils::Url &containerUrl);
        virtual void datasetAdded(const tempo_utils::Url &datasetUrl);
        virtual void datasetRemoved(const tempo_utils::Url &datasetUrl);
        virtual void mountCollectionFinished(
            const tempo_utils::Url &collectionUrl,
            const tempo_utils::Status &status);
    };

    class CreateContainerHandler {
    public:
        virtual ~CreateContainerHandler() = default;
        virtual void createContainerFinished(
            const tempo_utils::Url &containerUrl,
            const tempo_utils::Status &status);
    };

    class CreateDatasetHandler {
    public:
        virtual ~CreateDatasetHandler() = default;
        virtual void createDatasetFinished(
            const tempo_utils::Url &datasetUrl,
            const tempo_utils::Status &status);
    };

    class PutDataHandler {
    public:
        virtual ~PutDataHandler() = default;
        virtual void putDataFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::vector<std::string> &failedVectors,
            const tempo_utils::Status &status);
    };

    class RemoveDataHandler {
    public:
        virtual ~RemoveDataHandler() = default;
        virtual void removeDataFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const tempo_utils::Status &status);
    };

    class DeleteContainerHandler {
    public:
        virtual ~DeleteContainerHandler() = default;
        virtual void deleteContainerFinished(
            const tempo_utils::Url &containerUrl,
            const tempo_utils::Status &status);
    };

    class DeleteDatasetHandler {
    public:
        virtual ~DeleteDatasetHandler() = default;
        virtual void deleteDatasetFinished(
            const tempo_utils::Url &datasetUrl,
            const tempo_utils::Status &status);
    };

    class MountCollectionReactor : public grpc::ClientReadReactor<groove_mount::CollectionChange> {
    public:
        MountCollectionReactor(
            std::shared_ptr<StorageClient> client,
            tu_uint64 lsn,
            const std::string &collectionName,
            const tempo_config::ConfigMap &configMap,
            MountMode mode);

        tempo_utils::Status start(
            groove_mount::MountService::Stub *stub,
            std::shared_ptr<MountCollectionHandler> handler);
        tempo_utils::Status cancel();

        void OnReadInitialMetadataDone(bool ok) override;
        void OnReadDone(bool ok) override;
        void OnDone(const grpc::Status &status) override;

    private:
        std::shared_ptr<StorageClient> m_client;
        tu_uint64 m_lsn;
        groove_mount::MountCollectionRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::CollectionChange m_incoming;
        std::shared_ptr<MountCollectionHandler> m_handler;
        tempo_utils::Url m_collectionUrl;
    };

    class CreateContainerReactor : public grpc::ClientUnaryReactor {
    public:
        CreateContainerReactor(
            groove_mount::MountService::Stub *stub,
            const tempo_utils::Url &rootOrContainerUrl,
            const std::string &containerName,
            std::shared_ptr<CreateContainerHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        std::shared_ptr<CreateContainerHandler> m_handler;
        groove_mount::CreateContainerRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::CreateContainerResult m_result;
    };

    class CreateDatasetReactor : public grpc::ClientUnaryReactor {
    public:
        CreateDatasetReactor(
            groove_mount::MountService::Stub *stub,
            const tempo_utils::Url &containerUrl,
            const std::string &datasetName,
            const groove_model::GrooveSchema &schema,
            std::shared_ptr<CreateDatasetHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        std::shared_ptr<CreateDatasetHandler> m_handler;
        groove_mount::CreateDatasetRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::CreateDatasetResult m_result;
    };

    class PutDataReactor : public grpc::ClientUnaryReactor {
    public:
        PutDataReactor(
            groove_mount::MountService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseFrame> frame,
            std::shared_ptr<const arrow::Buffer> buffer,
            std::shared_ptr<PutDataHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
        std::shared_ptr<groove_data::BaseFrame> m_frame;
        std::shared_ptr<const arrow::Buffer> m_buffer;
        std::shared_ptr<PutDataHandler> m_handler;
        groove_mount::PutDataRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::PutDataResult m_result;
    };

    class DeleteDatasetReactor : public grpc::ClientUnaryReactor {
    public:
        DeleteDatasetReactor(
            groove_mount::MountService::Stub *stub,
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DeleteDatasetHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<DeleteDatasetHandler> m_handler;
        groove_mount::DeleteDatasetRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::DeleteDatasetResult m_result;
    };

    class DeleteContainerReactor : public grpc::ClientUnaryReactor {
    public:
        DeleteContainerReactor(
            groove_mount::MountService::Stub *stub,
            const tempo_utils::Url &containerUrl,
            std::shared_ptr<DeleteContainerHandler> handler);

        void OnDone(const grpc::Status &status) override;

    private:
        tempo_utils::Url m_containerUrl;
        std::shared_ptr<DeleteContainerHandler> m_handler;
        groove_mount::DeleteContainerRequest m_request;
        grpc::ClientContext m_context;
        groove_mount::DeleteContainerResult m_result;
    };

    class StorageClient : public std::enable_shared_from_this<StorageClient> {

    public:
        tempo_utils::Status connect();
        tempo_utils::Status shutdown();

        tempo_utils::Result<StorageIdentity> identify();

        tempo_utils::Result<tu_uint64> declareCollection(
            const std::string &collectionName,
            const tempo_config::ConfigMap &configMap,
            MountMode mode);
        tempo_utils::Status mountCollection(tu_uint64 lsn, std::shared_ptr<MountCollectionHandler> handler);
        tempo_utils::Status registerCollection(tu_uint64 lsn, const tempo_utils::Url &collectionUrl);
        tempo_utils::Status unmountCollection(tu_uint64 lsn);
        tempo_utils::Status releaseCollection(tu_uint64 lsn);

        tempo_utils::Result<tempo_utils::Url> createContainer(
            const tempo_utils::Url &rootOrContainerUrl,
            const std::string &containerName);
        tempo_utils::Status createContainer(
            const tempo_utils::Url &rootOrContainerUrl,
            const std::string &containerName,
            std::shared_ptr<CreateContainerHandler> handler);

        tempo_utils::Result<tempo_utils::Url> createDataset(
            const tempo_utils::Url &containerUrl,
            const std::string &datasetName,
            const groove_model::GrooveSchema &schema);
        tempo_utils::Status createDataset(
            const tempo_utils::Url &containerUrl,
            const std::string &datasetName,
            const groove_model::GrooveSchema &schema,
            std::shared_ptr<CreateDatasetHandler> handler);

        tempo_utils::Result<std::vector<std::string>> putData(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseFrame> frame);
        tempo_utils::Status putData(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseFrame> frame,
            std::shared_ptr<PutDataHandler> handler);

        tempo_utils::Status deleteDataset(const tempo_utils::Url &datasetUrl);
        tempo_utils::Status deleteDataset(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DeleteDatasetHandler> handler);

        tempo_utils::Status deleteContainer(const tempo_utils::Url &containerUrl);
        tempo_utils::Status deleteContainer(
            const tempo_utils::Url &containerUrl,
            std::shared_ptr<DeleteContainerHandler> handler);

        static tempo_utils::Result<std::shared_ptr<StorageClient>> create(
            const tempo_utils::Url &endpointUrl,
            std::shared_ptr<grpc::ChannelCredentials> credentials,
            const std::string &endpointServerName);

    private:
        tempo_utils::Url m_endpointUrl;
        std::shared_ptr<grpc::ChannelCredentials> m_credentials;
        std::string m_endpointServerName;
        std::shared_ptr<grpc::Channel> m_channel;
        std::unique_ptr<groove_mount::MountService::Stub> m_stub;

        absl::Mutex m_lock;
        absl::flat_hash_map<tu_uint64,MountCollectionReactor *> m_reactors ABSL_GUARDED_BY(m_lock);
        absl::flat_hash_map<tempo_utils::Url,tu_uint64> m_collectionToLsn ABSL_GUARDED_BY(m_lock);
        absl::flat_hash_map<tu_uint64,tempo_utils::Url> m_lsnToCollection ABSL_GUARDED_BY(m_lock);
        tu_uint64 m_lastLsn;

        StorageClient(
            const tempo_utils::Url &endpointUrl,
            std::shared_ptr<grpc::ChannelCredentials> credentials,
            const std::string &endpointServerName);
    };
}

#endif // GROOVE_STORAGE_STORAGE_CLIENT_H