
#include <groove/data_converter.h>
#include <groove/groove_convert_tool.h>
#include <groove/model_config_parser.h>
#include <groove/schema_builder.h>
#include <groove_io/dataset_writer.h>
#include <groove_model/groove_model.h>
#include <groove_model/schema_state.h>
#include <tempo_command/command_help.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_stream.h>

static const ExtensionConverter known_converters[] = {
    { DataFileType::Json, ".json", convert_json_input},
    { DataFileType::Unknown, nullptr, nullptr },            // sentinel value, must exist and be last!
};

tempo_utils::Status
groove_convert_tool(
    const std::filesystem::path &workspaceRoot,
    const std::filesystem::path &distributionRoot,
    tempo_command::TokenVector &tokens)
{
    TU_LOG_INFO << "invoking convert command";

    tempo_config::PathParser outputFileParser(std::filesystem::path{"dataset.gds"});
    tempo_config::UrlParser datasetUrlParser(tempo_utils::Url{});
    tempo_config::ConfigFileParser datasetFileParser(tempo_config::ConfigNode{});
    tempo_config::ConfigFileParser modelFileParser;
    tempo_config::SeqTParser<tempo_config::ConfigFile> modelFileListParser(&modelFileParser, {});
    tempo_config::ConfigStringParser modelDataParser;
    tempo_config::SeqTParser<tempo_config::ConfigNode> modelDataListParser(&modelDataParser, {});
    tempo_config::ConfigFileParser shapeFileParser;
    tempo_config::SeqTParser<tempo_config::ConfigFile> shapeFileListParser(&modelFileParser, {});
    tempo_config::ConfigStringParser shapeDataParser;
    tempo_config::SeqTParser<tempo_config::ConfigNode> shapeDataListParser(&modelDataParser, {});

    std::vector<tempo_command::Default> convertDefaults = {
        {"outputFile", outputFileParser.getDefault(), "the output file path", "PATH"},
        {"datasetUrl", {}, "use the specified dataset uri", "URI"},
        {"datasetFile", {}, "the dataset configuration file path", "PATH"},
        {"modelFileList", {}, "include the model described by the configuration file at the specified path", "PATH"},
        {"modelDataList", {}, "include the model described by the specified json", "JSON"},
        {"shapeFileList", {}, "include the shape described by the configuration file at the specified path", "PATH"},
        {"shapeDataList", {}, "include the shape described by the specified json", "JSON"},
    };

    std::vector<tempo_command::Grouping> convertGroupings = {
        {"outputFile", {"-o", "--output"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"datasetUrl", {"-u", "--dataset-uri"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"datasetFile", {"-d", "--dataset-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"modelFileList", {"-m", "--model-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"modelDataList", {"-M", "--model-config-data"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"shapeFileList", {"-s", "--shape-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"shapeDataList", {"-S", "--shape-config-data"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "outputFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "datasetUrl"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "datasetFile"},
        {tempo_command::MappingType::ANY_INSTANCES, "modelFileList"},
        {tempo_command::MappingType::ANY_INSTANCES, "modelDataList"},
        {tempo_command::MappingType::ANY_INSTANCES, "shapeFileList"},
        {tempo_command::MappingType::ANY_INSTANCES, "shapeDataList"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
    };

    tempo_command::OptionsHash convertOptions;
    tempo_command::ArgumentVector convertArguments;

    tempo_command::CommandConfig convertConfig = command_config_from_defaults(convertDefaults);

    // parse remaining options and arguments
    auto status = parse_completely(tokens, convertGroupings, convertOptions, convertArguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"groove", "convert"},
                    "Convert files to the groove dataset format",
                    {}, convertGroupings, optMappings, argMappings, convertDefaults);
            default:
                return status;
        }
    }

    // convert options to config
    status = convert_options(convertOptions, optMappings, convertConfig);
    if (!status.isOk())
        return status;

    // convert arguments to config
    status = convert_arguments(convertArguments, argMappings, convertConfig);
    if (!status.isOk())
        return status;

    TU_LOG_INFO << "convert config:\n" << tempo_command::command_config_to_string(convertConfig);

    // determine the output file
    std::filesystem::path outputFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(outputFile, outputFileParser,
        convertConfig, "outputFile"));

    // determine the dataset uri
    tempo_utils::Url datasetUrl;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(datasetUrl, datasetUrlParser,
        convertConfig, "datasetUrl"));
    if (!datasetUrl.isValid()) {
        datasetUrl = tempo_utils::Url::fromString(
            absl::StrCat("file://", absolute(outputFile).string()));
    }

    // parse the dataset config file
    tempo_config::ConfigFile datasetConfig;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(datasetConfig, datasetFileParser,
        convertConfig, "datasetFile"));

    // parse the list of model configs
    std::vector<tempo_config::ConfigNode> modelConfigList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(modelConfigList, modelDataListParser,
        convertConfig, "modelDataList"));

    // parse the list of model config files
    std::vector<tempo_config::ConfigFile> modelFileList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(modelFileList, modelFileListParser,
        convertConfig, "modelFileList"));

    // parse the list of shape configs
    std::vector<tempo_config::ConfigNode> shapeConfigList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(shapeConfigList, shapeDataListParser,
        convertConfig, "shapeDataList"));

    // parse the list of shape config files
    std::vector<tempo_config::ConfigFile> shapeFileList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(shapeFileList, shapeFileListParser,
        convertConfig, "shapeFileList"));

    // construct the map of models from the combination of input config and model configs
    absl::flat_hash_map<std::string,ModelConfig> modelConfigs;

    // parse each model config file argument
    for (const auto &configFile : modelFileList) {
        auto rootNode = configFile.getRoot();
        if (rootNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "invalid model config file {}", configFile.getPath().string());
        auto configMap = rootNode.toMap();

        tempo_config::StringParser modelIdParser;
        std::string modelId;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelId, modelIdParser,
            configMap, "modelId"));
        if (modelConfigs.contains(modelId))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "model '{}' is already defined", modelId);

        const auto &dataRoot = configFile.getPath().parent_path();
        status = update_model_configs(modelConfigs, modelId, configMap, dataRoot);
        if (status.notOk())
            return status;
    }

    // parse each model config data argument
    for (const auto &rootNode : modelConfigList) {
        if (rootNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "invalid model config data");
        auto configMap = rootNode.toMap();

        tempo_config::StringParser modelIdParser;
        std::string modelId;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(modelId, modelIdParser,
            configMap, "modelId"));
        if (modelConfigs.contains(modelId))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "model '{}' is already defined", modelId);

        status = update_model_configs(modelConfigs, modelId, configMap, std::filesystem::current_path());
        if (status.notOk())
            return status;
    }

    // generate the schema
    auto makeSchemaResult = make_schema(modelConfigs);
    if (makeSchemaResult.isStatus())
        return makeSchemaResult.getStatus();
    auto schema = makeSchemaResult.getResult();

    groove_io::DatasetWriter writer(datasetUrl, schema);

    // convert each model to a frame and write it to the dataset
    for (const auto &modelEntry : modelConfigs) {
        const auto &modelId = modelEntry.first;
        const auto &modelConfig = modelEntry.second;
        const auto dataPath = modelConfig.dataPath;

        // determine which converter to use
        ExtensionConverterFunc converter = nullptr;
        for (auto *curr = known_converters; curr->extension != nullptr; curr++) {
            if (modelConfig.type != DataFileType::Unknown && curr->type == modelConfig.type) {
                converter = curr->converter;
                break;
            }
            if (dataPath.extension() == curr->extension) {
                converter = curr->converter;
                break;
            }
        }
        if (converter == nullptr)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "unhandled file extension ", dataPath.extension().string());

        // invoke the extension-specific converter
        auto convertInputResult = converter(modelConfig, dataPath);
        if (convertInputResult.isStatus())
            return convertInputResult.getStatus();
        auto frame = convertInputResult.getResult();

        // write the frame to the dataset
        auto ioStatus = writer.putFrame(modelId, frame);
        if (ioStatus.notOk())
            return ioStatus;
    }

    // build the dataset
    auto ioStatus = writer.writeDataset(outputFile);
    if (ioStatus.notOk())
        return ioStatus;

    return tempo_command::CommandStatus::ok();
}