#ifndef GROOVE_IO_DATASET_FILE_H
#define GROOVE_IO_DATASET_FILE_H

#include <filesystem>

#include <groove_model/abstract_dataset.h>

#include "dataset_reader.h"

namespace groove_io {

    class DatasetFile : public groove_model::AbstractDataset {

    public:
        bool isValid() const;

        std::filesystem::path getDatasetPath() const;

        tempo_utils::Url getDatasetUrl() const override;
        groove_model::GrooveSchema getSchema() const override;
        bool isImmutable() const override;
        bool hasModel(const std::string &modelId) const override;
        std::shared_ptr<groove_model::GrooveModel> getModel(const std::string &modelId) const override;

        static tempo_utils::Result<std::shared_ptr<DatasetFile>> create(
            const std::filesystem::path &datasetPath,
            const tempo_utils::Url &overrideUrl = {});

    private:
        tempo_utils::Url m_datasetUrl;
        absl::flat_hash_map<std::string, std::shared_ptr<groove_model::GrooveModel>> m_models;
        std::shared_ptr<DatasetReader> m_reader;

        DatasetFile(
            tempo_utils::Url datasetUrl,
            const absl::flat_hash_map<std::string, std::shared_ptr<groove_model::GrooveModel>> &models,
            std::shared_ptr<DatasetReader> reader);
    };
}

#endif // GROOVE_IO_DATASET_FILE_H
