
#include <groove_agent/dataset_config_parser.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>

tempo_utils::Status
update_dataset_configs(
    absl::flat_hash_map<tempo_utils::Url,DatasetConfig> &datasetConfigs,
    const tempo_utils::Url &datasetUrl,
    const tempo_config::ConfigMap &configMap,
    const std::filesystem::path &dataRoot)
{
    tempo_config::UrlParser storageUrlParser;
    tempo_config::PathParser dataPathParser;

    DatasetConfig datasetConfig;
    datasetConfig.datasetUrl = datasetUrl;

    std::filesystem::path path(datasetUrl.getPath(), std::filesystem::path::generic_format);
    if (path.begin() == path.end())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "invalid path for dataset url {}", datasetUrl.toString());
    datasetConfig.collectionName = std::string(*path.begin());

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(datasetConfig.dataPath, dataPathParser,
        configMap, "dataPath"));

    if (datasetConfig.dataPath.is_relative()) {
        datasetConfig.dataPath = dataRoot / datasetConfig.dataPath;
    }
    if (!exists(datasetConfig.dataPath))
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "missing data file at {}", datasetConfig.dataPath.string());

    datasetConfigs.insert_or_assign(datasetConfig.datasetUrl, std::move(datasetConfig));

    return tempo_command::CommandStatus::ok();
}
