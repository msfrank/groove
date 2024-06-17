
#include <groove/model_config_parser.h>
#include <tempo_command/command_result.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>

tempo_utils::Status
update_column_configs(
    absl::flat_hash_map<std::string,ColumnConfig> &columnConfigs,
    const std::string &columnId,
    const tempo_config::ConfigMap &configMap)
{
    tempo_config::EnumTParser<groove_data::DataValueType> valueTypeParser({
        {"Double", groove_data::DataValueType::VALUE_TYPE_DOUBLE},
        {"Int64", groove_data::DataValueType::VALUE_TYPE_INT64},
        {"String", groove_data::DataValueType::VALUE_TYPE_STRING},
    }, groove_data::DataValueType::VALUE_TYPE_UNKNOWN);
    tempo_config::BooleanParser nullsAllowedParser(false);
    tempo_config::StringParser fidelityFieldParser(std::string{});

    ColumnConfig columnConfig;
    columnConfig.columnId = columnId;

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(columnConfig.valueType, valueTypeParser,
        configMap, "valueType"));

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(columnConfig.nullsAllowed, nullsAllowedParser,
        configMap, "nullsAllowed"));

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(columnConfig.fidelityField, fidelityFieldParser,
        configMap, "fidelityField"));

    columnConfigs.insert_or_assign(columnConfig.columnId, std::move(columnConfig));

    return tempo_command::CommandStatus::ok();
}

tempo_utils::Status
update_model_configs(
    absl::flat_hash_map<std::string,ModelConfig> &modelConfigs,
    const std::string &modelId,
    const tempo_config::ConfigMap &configMap,
    const std::filesystem::path &dataRoot)
{
    tempo_config::PathParser dataPathParser;
    tempo_config::StringParser keyFieldParser;
    tempo_config::EnumTParser<groove_data::DataKeyType> keyTypeParser({
        {"Category", groove_data::DataKeyType::KEY_CATEGORY},
        {"Double", groove_data::DataKeyType::KEY_DOUBLE},
        {"Int64", groove_data::DataKeyType::KEY_INT64},
    }, groove_data::DataKeyType::KEY_UNKNOWN);
    tempo_config::EnumTParser<groove_data::CollationMode> collationModeParser({
        {"Indexed", groove_data::CollationMode::COLLATION_INDEXED},
        {"Sorted", groove_data::CollationMode::COLLATION_SORTED},
    }, groove_data::CollationMode::COLLATION_UNKNOWN);
    tempo_config::EnumTParser<DataFileType> dataFileTypeParser({
        {"Csv", DataFileType::Csv},
        {"Json", DataFileType::Json},
    }, DataFileType::Unknown);

    ModelConfig modelConfig;
    modelConfig.modelId = modelId;

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelConfig.keyField, keyFieldParser,
        configMap, "keyField"));

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelConfig.keyType, keyTypeParser,
        configMap, "keyType"));

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelConfig.collation, collationModeParser,
        configMap, "collationMode"));

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelConfig.dataPath, dataPathParser,
        configMap, "dataPath"));

    if (modelConfig.dataPath.is_relative()) {
        modelConfig.dataPath = dataRoot / modelConfig.dataPath;
    }
    if (!exists(modelConfig.dataPath))
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "missing data file at {}", modelConfig.dataPath.string());

    if (!configMap.mapContains("columns"))
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "model has no columns defined");
    auto modelsNode = configMap.mapAt("columns");
    if (modelsNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "model has no columns defined");
    auto modelsMap = modelsNode.toMap();

    for (auto iterator = modelsMap.mapBegin(); iterator != modelsMap.mapEnd(); iterator++) {
        const auto &columnId = iterator->first;
        if (modelConfig.columns.contains(columnId))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "column '{}' is already defined for model", columnId);

        const auto &columnNode = iterator->second;
        if (columnNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_config::ConfigStatus::forCondition(
                tempo_config::ConfigCondition::kWrongType, "invalid column config");
        auto columnMap = columnNode.toMap();
        auto status = update_column_configs(modelConfig.columns, columnId, columnMap);
        if (status.notOk())
            return status;
    }

    if (configMap.mapContains("dataConverter")) {
        auto dataConverterMap = configMap.mapAt("dataConverter").toMap();
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelConfig.type, dataFileTypeParser,
            dataConverterMap.toMap(), "dataType"));
        if (dataConverterMap.mapContains("parameters")) {
            modelConfig.convertParameters = dataConverterMap.mapAt("parameters");
        }
    }

    modelConfigs.insert_or_assign(modelConfig.modelId, std::move(modelConfig));

    return tempo_command::CommandStatus::ok();
}
