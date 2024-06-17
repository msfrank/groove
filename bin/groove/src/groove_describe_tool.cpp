
#include <groove/groove_describe_tool.h>
#include <groove_io/dataset_reader.h>
#include <groove_io/dataset_writer.h>
#include <groove_model/groove_model.h>
#include <tempo_command/command_help.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_helpers.h>

static tempo_command::CommandStatus
print_model(
    const groove_model::ModelWalker &model,
    int indent)
{
    TU_CONSOLE_OUT << tempo_utils::Indent(indent) << model.getModelId();
    indent += 4;

    std::string keyTypeName;
    switch (model.getKeyType()) {
        case groove_model::ModelKeyType::Category:
            keyTypeName = "Category";
            break;
        case groove_model::ModelKeyType::Double:
            keyTypeName = "Double";
            break;
        case groove_model::ModelKeyType::Int64:
            keyTypeName = "Int64";
            break;
        default:
            keyTypeName = "???";
            break;
    }
    TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Key Type: " << keyTypeName;

    std::string keyCollationName;
    switch (model.getKeyCollation()) {
        case groove_model::ModelKeyCollation::Sorted:
            keyCollationName = "Sorted";
            break;
        case groove_model::ModelKeyCollation::Indexed:
            keyCollationName = "Indexed";
            break;
        default:
            keyCollationName = "???";
            break;
    }
    TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Key Collation: " << keyCollationName;

    if (model.numColumns() > 0) {
        TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Columns:";

        for (int i = 0; i < model.numColumns(); i++) {
            const auto column = model.getColumn(i);
            if (!column.isValid())
                return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                    "schema contains invalid column");
            indent += 4;
            TU_CONSOLE_OUT << tempo_utils::Indent(indent) << column.getColumnId();
            indent += 4;

            std::string valueTypeName;
            switch (column.getValueType()) {
                case groove_model::ColumnValueType::Double:
                    valueTypeName = "Double";
                    break;
                case groove_model::ColumnValueType::Int64:
                    valueTypeName = "Int64";
                    break;
                case groove_model::ColumnValueType::String:
                    valueTypeName = "String";
                    break;
                default:
                    valueTypeName = "???";
                    break;
            }
            TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Value Type: " << valueTypeName;

            std::string valueFidelityName;
            switch (column.getValueFidelity()) {
                case groove_model::ColumnValueFidelity::OnlyValidValue:
                    valueFidelityName = "OnlyValidValue";
                    break;
                case groove_model::ColumnValueFidelity::OnlyValidOrEmpty:
                    valueFidelityName = "OnlyValidOrEmpty";
                    break;
                case groove_model::ColumnValueFidelity::AnyFidelityAllowed:
                    valueFidelityName = "AnyFidelityAllowed";
                    break;
                default:
                    valueFidelityName = "???";
                    break;
            }
            TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Fidelity: " << valueFidelityName;
        }
    } else {
        TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Columns: none";
    }

    return tempo_command::CommandStatus::ok();
}

tempo_utils::Status
groove_describe_tool(
    const std::filesystem::path &workspaceRoot,
    const std::filesystem::path &distributionRoot,
    tempo_command::TokenVector &tokens)
{
    TU_LOG_INFO << "invoking describe command";

    tempo_config::PathParser datasetPathParser;
    tempo_config::SeqTParser<std::filesystem::path> datasetPathListParser(&datasetPathParser);

    std::vector<tempo_command::Default> describeDefaults = {
        {"datasetPathList", {}, "path to the dataset to describe", "PATH"},
    };

    std::vector<tempo_command::Grouping> describeGroupings = {
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
    };

    std::vector<tempo_command::Mapping> optMappings = {
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ANY_INSTANCES, "datasetPathList"},
    };

    tempo_command::OptionsHash describeOptions;
    tempo_command::ArgumentVector describeArguments;

    tempo_command::CommandConfig describeConfig = command_config_from_defaults(describeDefaults);

    // parse remaining options and arguments
    auto status = parse_completely(tokens, describeGroupings, describeOptions, describeArguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"groove", "describe"},
                    "Print information about the specified groove dataset(s)",
                    {}, describeGroupings, optMappings, argMappings, describeDefaults);
            default:
                return status;
        }
    }

    // convert options to config
    status = convert_options(describeOptions, optMappings, describeConfig);
    if (!status.isOk())
        return status;

    // convert arguments to config
    status = convert_arguments(describeArguments, argMappings, describeConfig);
    if (!status.isOk())
        return status;

    TU_LOG_INFO << "describe config:\n" << tempo_command::command_config_to_string(describeConfig);

    // determine the list of dataset files
    std::vector<std::filesystem::path> datasetPathList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(datasetPathList, datasetPathListParser,
        describeConfig, "datasetPathList"));

    // describe each dataset file
    for (const auto &datasetPath : datasetPathList) {
        auto createReaderResult = groove_io::DatasetReader::create(datasetPath);
        if (createReaderResult.isStatus())
            return createReaderResult.getStatus();
        auto reader = createReaderResult.getResult();
        if (!reader->isValid())
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "file {} is not a valid dataset", datasetPath.string());

        TU_CONSOLE_OUT << "Dataset: " << canonical(absolute(datasetPath));

        int indent = 4;
        TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Version: " << reader->getVersion();
        TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Flags: " << reader->getFlags();

        auto schema = reader->getSchema().getSchema();
        TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Schema ABI: " << reader->getVersion();

        if (schema.numModels() > 0) {
            TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Models:";
            for (int i = 0; i < schema.numModels(); i++) {
                const auto model = schema.getModel(i);
                if (!model.isValid())
                    return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                        "schema contains invalid model");
                auto printStatus = print_model(model, indent + 4);
                if (printStatus.notOk())
                    return printStatus;
            }
        } else {
            TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "Models: none";
        }
    }

    return tempo_command::CommandStatus::ok();
}