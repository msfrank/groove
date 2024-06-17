#include <iostream>

#include <groove/groove_convert_tool.h>
#include <groove/groove_describe_tool.h>
#include <tempo_command/command_config.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_command/command_tokenizer.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>

inline tempo_utils::Status
run_tool(
    const std::vector<tempo_command::Command> &commands,
    const std::string &subcommand,
    const std::string &workspaceRoot,
    const std::string &distributionRoot,
    tempo_command::TokenVector &tokens)
{
    for (const auto &command : commands) {
        if (command.name == subcommand)
            return command.func(workspaceRoot, distributionRoot, tokens);
    }
    return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kUnknownSubcommand,
        "unknown subcommand '{}'", subcommand);
}

static std::filesystem::path
find_workspace_root(const std::string &currentDirectoryPath)
{
    auto currentDir = std::filesystem::current_path();

    for (;;) {
        auto workspacePath = currentDir / "groove.workspace";
        if (std::filesystem::exists(workspacePath))
            return currentDir;
        if (currentDir == currentDir.root_path())
            break;
        currentDir = currentDir.parent_path();
    }
    return {};
}

tempo_utils::Status
run_groove(int argc, const char *argv[])
{
    const std::vector<tempo_command::Command> globalCommands = {
        {"convert", groove_convert_tool, "Convert files to the groove dataset format"},
        {"describe", groove_describe_tool, "Print information about the specified groove dataset(s)"},
    };

    tempo_config::PathParser workspaceRootParser(
        find_workspace_root(std::filesystem::current_path()));
    tempo_config::PathParser distributionRootParser(std::filesystem::path{});
    tempo_config::BooleanParser colorizeOutputParser(false);
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    std::vector<tempo_command::Default> globalDefaults = {
        {"workspaceRoot", workspaceRootParser.getDefault(),
            "Specify an alternative workspace root directory", "DIR"},
        {"distributionRoot", distributionRootParser.getDefault(),
            "Specify an alternative distribution root directory", "DIR"},
        {"colorizeOutput", colorizeOutputParser.getDefault(),
            "Display colorized output"},
        {"verbose", verboseParser.getDefault(),
            "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", quietParser.getDefault(),
            "Display warnings and errors only (specify twice for errors only)"},
        {"silent", silentParser.getDefault(),
            "Suppress all output"},
    };

    const std::vector<tempo_command::Grouping> globalGroupings = {
        {"workspaceRoot", {"-W", "--workspace-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"distributionRoot", {"--distribution-root"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"colorizeOutput", {"-c", "--colorize"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    const std::vector<tempo_command::Mapping> globalMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "workspaceRoot"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "distributionRoot"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "colorizeOutput"},
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    // parse argv array into a vector of tokens
    auto tokenizeResult = tempo_command::tokenize_argv(argc - 1, &argv[1]);
    if (tokenizeResult.isStatus())
        display_status_and_exit(tokenizeResult.getStatus());
    auto tokens = tokenizeResult.getResult();

    std::string subcommand;
    tempo_command::OptionsHash globalOptions;

    // parse global options and get the subcommand
    auto status = parse_until_subcommand(tokens, globalCommands, globalGroupings,
        subcommand, globalOptions);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                display_help_and_exit({"groove"},
                    "Convert, transform, and otherwise manipulate data",
                    globalCommands, globalGroupings, globalMappings,
                    {}, globalDefaults);
            case tempo_command::CommandCondition::kVersionRequested:
                tempo_command::display_version_and_exit(FULL_VERSION);
            default:
                return status;
        }
    }

    // initialize the global config from defaults
    tempo_command::CommandConfig globalConfig = command_config_from_defaults(globalDefaults);

    // convert to command config
    status = convert_options(globalOptions, globalMappings, globalConfig);
    if (!status.isOk())
        return status;

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        true,
        false,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(silent, silentParser,
        globalConfig, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(verbose, verboseParser,
            globalConfig, "verbose"));
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(quiet, quietParser,
            globalConfig, "quiet"));
        if (verbose && quiet)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "cannot specify both -v and -q");
        if (verbose == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVerbose;
        } else if (verbose > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        }
        if (quiet == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kWarningsAndErrors;
        } else if (quiet > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kErrorsOnly;
        }
    }
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(logging.colorizeOutput, colorizeOutputParser,
        globalConfig, "colorizeOutput"));

    // initialize logging
    tempo_utils::init_logging(logging);

    TU_LOG_INFO << "global config:\n" << tempo_command::command_config_to_string(globalConfig);

    // determine the workspace root
    std::filesystem::path workspaceRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(workspaceRoot, workspaceRootParser,
        globalConfig, "workspaceRoot"));

    // determine the distribution root
    std::filesystem::path distributionRoot;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(distributionRoot, distributionRootParser,
        globalConfig, "distributionRoot"));

    // pass control the appropriate tool
    return run_tool(globalCommands, subcommand, workspaceRoot, distributionRoot, tokens);
}