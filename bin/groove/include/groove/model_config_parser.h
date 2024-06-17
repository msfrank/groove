#ifndef GROOVE_MODEL_CONFIG_PARSER_H
#define GROOVE_MODEL_CONFIG_PARSER_H

#include <string>
#include <filesystem>

#include <groove_data/data_types.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/status.h>

enum class DataFileType {
    Unknown,
    Csv,
    Json,
};

struct ColumnConfig {
    std::string columnId;
    groove_data::DataValueType valueType;
    bool nullsAllowed;
    std::string fidelityField;
};

struct ModelConfig {
    std::string modelId;
    std::filesystem::path dataPath;
    DataFileType type;
    std::string keyField;
    groove_data::DataKeyType keyType;
    groove_data::CollationMode collation;
    absl::flat_hash_map<std::string,ColumnConfig> columns;
    tempo_config::ConfigNode convertParameters;
};

tempo_utils::Status
update_column_configs(
    absl::flat_hash_map<std::string,ColumnConfig> &columnConfigs,
    const std::string &columnId,
    const tempo_config::ConfigMap &configMap);

tempo_utils::Status
update_model_configs(
    absl::flat_hash_map<std::string,ModelConfig> &modelConfigs,
    const std::string &modelId,
    const tempo_config::ConfigMap &configMap,
    const std::filesystem::path &dataRoot);

#endif // GROOVE_MODEL_CONFIG_PARSER_H