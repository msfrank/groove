#ifndef GROOVE_AGENT_DATASET_CONFIG_PARSER_H
#define GROOVE_AGENT_DATASET_CONFIG_PARSER_H

#include <string>
#include <filesystem>

#include <groove_data/data_types.h>
#include <tempo_command/command_result.h>
#include <tempo_config/config_types.h>
#include <tempo_utils/url.h>

struct DatasetConfig {
    std::string collectionName;
    tempo_utils::Url datasetUrl;
    std::filesystem::path dataPath;
};

tempo_utils::Status
update_dataset_configs(
    absl::flat_hash_map<tempo_utils::Url,DatasetConfig> &datasetConfigs,
    const tempo_utils::Url &datasetUrl,
    const tempo_config::ConfigMap &configMap,
    const std::filesystem::path &dataRoot);

#endif // GROOVE_AGENT_DATASET_CONFIG_PARSER_H