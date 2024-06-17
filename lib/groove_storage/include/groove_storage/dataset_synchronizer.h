#ifndef GROOVE_STORAGE_DATASET_SYNCHRONIZER_H
#define GROOVE_STORAGE_DATASET_SYNCHRONIZER_H

#include <groove_model/groove_database.h>

#include "syncing_client.h"

namespace groove_storage {

    class SchemaSyncNotification {
    public:
        virtual ~SchemaSyncNotification() = default;
        virtual void schemaSyncFinished(
            const tempo_utils::Url &datasetUrl,
            const groove_model::GrooveSchema &schema,
            const tempo_utils::Status &status);
    };

    class ModelSyncNotification {
    public:
        virtual ~ModelSyncNotification() = default;
        virtual void modelSyncFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const tempo_utils::Status &status);
    };

    class DatasetSynchronizer : public std::enable_shared_from_this<DatasetSynchronizer> {

    public:
        bool isValid() const;

        tempo_utils::Status synchronizeSchema(std::shared_ptr<SchemaSyncNotification> notifier);

        tempo_utils::Status synchronizeModel(
            const std::string &modelId,
            const groove_data::CategoryRange &range,
            std::shared_ptr<ModelSyncNotification> notifier);
        tempo_utils::Status synchronizeModel(
            const std::string &modelId,
            const groove_data::DoubleRange &range,
            std::shared_ptr<ModelSyncNotification> notifier);
        tempo_utils::Status synchronizeModel(
            const std::string &modelId,
            const groove_data::Int64Range &range,
            std::shared_ptr<ModelSyncNotification> notifier);

        static tempo_utils::Result<std::shared_ptr<DatasetSynchronizer>>
        create(
            std::shared_ptr<groove_model::GrooveDatabase> db,
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<SyncingClient> client);

    private:
        std::shared_ptr<groove_model::GrooveDatabase> m_db;
        tempo_utils::Url m_datasetUrl;
        groove_model::GrooveSchema m_schema;
        std::shared_ptr<SyncingClient> m_client;
        absl::Mutex m_lock;

        DatasetSynchronizer(
            std::shared_ptr<groove_model::GrooveDatabase> db,
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<SyncingClient> client);

        void updateSchema(groove_model::GrooveSchema schema);
        void updateModel(const std::string &modelId, std::shared_ptr<groove_data::BaseFrame> frame);

        friend class SynchronizeSchemaHandler;
        friend class SynchronizeModelHandler;
    };

    class SynchronizeSchemaHandler
        : public DescribeDatasetHandler,
          public std::enable_shared_from_this<SynchronizeSchemaHandler> {

    public:
        void describeDatasetFinished(
            const tempo_utils::Url &datasetUrl,
            const groove_model::GrooveSchema &schema,
            const tempo_utils::Status &status) override;

        static std::shared_ptr<SynchronizeSchemaHandler>
        create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DatasetSynchronizer> synchronizer,
            std::shared_ptr<SyncingClient> client,
            std::shared_ptr<SchemaSyncNotification> notifier);

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<DatasetSynchronizer> m_synchronizer;
        std::shared_ptr<SyncingClient> m_client;
        std::shared_ptr<SchemaSyncNotification> m_notifier;

        SynchronizeSchemaHandler(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<DatasetSynchronizer> synchronizer,
            std::shared_ptr<SyncingClient> client,
            std::shared_ptr<SchemaSyncNotification> notifier);
    };

    class SynchronizeModelHandler
        : public GetShardsHandler,
          public GetDataHandler,
          public std::enable_shared_from_this<SynchronizeModelHandler> {

    public:
        void getShardsFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const std::vector<std::string> &shards,
            const tempo_utils::Status &status) override;
        void categoryFrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::CategoryFrame> frame) override;
        void doubleFrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::DoubleFrame> frame) override;
        void int64FrameReceived(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::Int64Frame> frame) override;
        void getDataFinished(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const tempo_utils::Status &status) override;

        static std::shared_ptr<SynchronizeModelHandler>
        create(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<DatasetSynchronizer> synchronizer,
            std::shared_ptr<SyncingClient> client,
            std::shared_ptr<ModelSyncNotification> notifier);

    private:
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
        std::shared_ptr<DatasetSynchronizer> m_synchronizer;
        std::shared_ptr<SyncingClient> m_client;
        std::shared_ptr<ModelSyncNotification> m_notifier;
        std::forward_list<std::string> m_shards;

        SynchronizeModelHandler(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<DatasetSynchronizer> synchronizer,
            std::shared_ptr<SyncingClient> client,
            std::shared_ptr<ModelSyncNotification> notifier);
    };
}

#endif // GROOVE_STORAGE_DATASET_SYNCHRONIZER_H