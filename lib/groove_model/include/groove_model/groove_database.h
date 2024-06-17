#ifndef GROOVE_MODEL_GROOVE_DATABASE_H
#define GROOVE_MODEL_GROOVE_DATABASE_H

#include <filesystem>
#include <string>

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

#include <tempo_utils/url.h>

#include "abstract_dataset.h"
#include "groove_schema.h"
#include "groove_model.h"
#include "model_result.h"
#include "rocksdb_store.h"

namespace groove_model {

    struct DatabaseOptions {
        std::filesystem::path modelsDirectory;
        tu_int64 cacheSizeInBytes = -1;
    };

    class DatabaseDataset : public AbstractDataset {
    public:
        DatabaseDataset(
            const tempo_utils::Url &datasetUrl,
            const GrooveSchema &schema,
            const absl::flat_hash_map<std::string, std::shared_ptr<GrooveModel>> &models);

        tempo_utils::Url getDatasetUrl() const override;
        groove_model::GrooveSchema getSchema() const override;
        bool isImmutable() const override;
        bool hasModel(const std::string &modelId) const override;
        std::shared_ptr<groove_model::GrooveModel> getModel(const std::string &modelId) const override;

    private:
        tempo_utils::Url m_datasetUrl;
        GrooveSchema m_schema;
        absl::flat_hash_map<std::string, std::shared_ptr<GrooveModel>> m_models;
    };

    class GrooveDatabase {

    public:
        explicit GrooveDatabase(const DatabaseOptions &options = {});
        explicit GrooveDatabase(std::shared_ptr<const std::string> databaseId, const DatabaseOptions &options = {});
        ~GrooveDatabase();

        tempo_utils::Status configure();

        tempo_utils::Status declareDataset(const tempo_utils::Url &datasetUrl, const GrooveSchema &schema);
        bool hasDataset(const tempo_utils::Url &datasetUrl) const;
        std::shared_ptr<AbstractDataset> getDataset(const tempo_utils::Url &datasetUrl) const;
        tempo_utils::Status dropDataset(const tempo_utils::Url &datasetUrl);

        GrooveSchema getSchema(const tempo_utils::Url &datasetUrl) const;

        tempo_utils::Status updateModel(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseFrame> frame,
            std::vector<std::string> *failedVectors = nullptr);

    private:
        DatabaseOptions m_options;
        std::shared_ptr<const std::string> m_databaseId;
        std::filesystem::path m_dbDirectory;
        std::shared_ptr<RocksDbStore> m_store;
        absl::node_hash_map<tempo_utils::Url, std::shared_ptr<DatabaseDataset>> m_datasets;
        absl::Mutex *m_lock;
    };
}

#endif // GROOVE_MODEL_GROOVE_DATABASE_H