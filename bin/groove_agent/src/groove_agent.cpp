#include <vector>
#include <sys/fcntl.h>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <uv.h>

#include <grpcpp/ext/channelz_service_plugin.h>
#include <gens/src/proto/grpc/channelz/channelz.grpc.pb.h>

#include <groove_agent/channel_monitor.h>
#include <groove_agent/dataset_config_parser.h>
#include <groove_agent/mount_service.h>
#include <groove_agent/sync_service.h>
#include <tempo_command/command_help.h>
#include <tempo_command/command_parser.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/posix_result.h>

static void on_termination_signal(uv_signal_t *handle, int signal)
{
    TU_LOG_INFO << "caught signal " << signal;
    uv_stop(handle->loop);
}

static std::shared_ptr<grpc::ServerCredentials>
make_ssl_server_credentials(
    const std::filesystem::path &pemCertificateFile,
    const std::filesystem::path &pemPrivateKeyFile,
    const std::filesystem::path &pemRootCABundleFile)
{
    tempo_utils::FileReader certificateReader(pemCertificateFile);
    if (certificateReader.getStatus().notOk())
        return {};
    tempo_utils::FileReader privateKeyReader(pemPrivateKeyFile);
    if (privateKeyReader.getStatus().notOk())
        return {};
    tempo_utils::FileReader rootCABundleReader(pemRootCABundleFile);
    if (rootCABundleReader.getStatus().notOk())
        return {};

    grpc::SslServerCredentialsOptions options;
    grpc::SslServerCredentialsOptions::PemKeyCertPair pair;
    auto rootCABytes = rootCABundleReader.getBytes();
    auto certificateBytes = certificateReader.getBytes();
    auto privateKeyBytes = privateKeyReader.getBytes();
    options.pem_root_certs = std::string((const char *) rootCABytes->getData(), rootCABytes->getSize());
    pair.cert_chain = std::string((const char *) certificateBytes->getData(), certificateBytes->getSize());
    pair.private_key = std::string((const char *) privateKeyBytes->getData(), privateKeyBytes->getSize());
    options.pem_key_cert_pairs.push_back(pair);

    return grpc::SslServerCredentials(options);
}

tempo_utils::Status
run_zuri_storage_agent(int argc, const char *argv[])
{
    char hostname[256];
    memset(hostname, 0, 256);
    size_t len = 256;
    uv_os_gethostname(hostname, &len);
    tempo_config::StringParser agentNameParser(absl::StrCat(getpid(), "@", hostname));

    tempo_config::UrlParser listenerUrlParser(tempo_utils::Url{});
    tempo_config::PathParser pemCertificateFileParser(std::filesystem::path{});
    tempo_config::PathParser pemPrivateKeyFileParser(std::filesystem::path{});
    tempo_config::PathParser pemRootCABundleFileParser(std::filesystem::path{});
    tempo_config::BooleanParser runInBackgroundParser(false);
    tempo_config::BooleanParser temporarySessionParser(false);
    tempo_config::StringParser transportParser(std::string{});
    tempo_config::IntegerParser idleTimeoutParser(0);
    tempo_config::BooleanParser emitEndpointParser(false);
    tempo_config::PathParser logFileParser(std::filesystem::path(
        absl::StrCat("groove-agent.", getpid(), ".log")));
    tempo_config::PathParser pidFileParser(std::filesystem::path{});
    tempo_config::ConfigFileParser datasetFileParser;
    tempo_config::SeqTParser<tempo_config::ConfigFile> datasetFileListParser(&datasetFileParser, {});
    tempo_config::ConfigStringParser datasetDataParser;
    tempo_config::SeqTParser<tempo_config::ConfigNode> datasetDataListParser(&datasetDataParser, {});

    std::vector<tempo_command::Default> defaults = {
        {"listenerUrl", {}, "listen on the specified endpoint url", "URL"},
        {"pemCertificateFile", {}, "the certificate used by gRPC", "FILE"},
        {"pemPrivateKeyFile", {}, "the private key used by gRPC", "FILE"},
        {"pemRootCABundleFile", {}, "the root CA certificate bundle used by gRPC", "FILE"},
        {"runInBackground", {}, "run agent in the background", {}},
        {"temporarySession", {}, "agent will shutdown automatically after a period of inactivity", {}},
        {"transport", {}, "if listener url is not specified, then create listener using the specified type", "TYPE"},
        {"idleTimeout", {}, "shutdown the agent after the specified amount of time has elapsed", "SECONDS"},
        {"emitEndpoint", {}, "print the endpoint url after initialization has completed", {}},
        {"logFile", {}, "path to log file", "FILE"},
        {"pidFile", {}, "record the agent process id in the specified pid file", "FILE"},
        {"datasetFileList", {}, "include the dataset described by the configuration file at the specified path", "PATH"},
        {"datasetDataList", {}, "include the dataset described by the specified json", "JSON"},
    };

    std::vector<tempo_command::Grouping> groupings = {
        {"listenerUrl", {"-l", "--listener-url"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"pemCertificateFile", {"--certificate"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"pemPrivateKeyFile", {"--private-key"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"pemRootCABundleFile", {"--ca-bundle"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"runInBackground", {"--background"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"temporarySession", {"--temporary-session"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"transport", {"--transport"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"idleTimeout", {"--idle-timeout"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"emitEndpoint", {"--emit-endpoint"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"logFile", {"--log-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"pidFile", {"--pid-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"datasetFileList", {"-d", "--dataset-config-file"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"datasetDataList", {"-D", "--dataset-config-data"}, tempo_command::GroupingType::SINGLE_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "listenerUrl"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "pemCertificateFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "pemPrivateKeyFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "pemRootCABundleFile"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "runInBackground"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "temporarySession"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "transport"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "idleTimeout"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "emitEndpoint"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "logFile"},
        {tempo_command::MappingType::ZERO_OR_ONE_INSTANCE, "pidFile"},
        {tempo_command::MappingType::ANY_INSTANCES, "datasetFileList"},
        {tempo_command::MappingType::ANY_INSTANCES, "datasetDataList"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
    };

    tempo_command::OptionsHash options;
    tempo_command::ArgumentVector arguments;

    tempo_command::CommandConfig config = command_config_from_defaults(defaults);

    // parse argv array into a vector of tokens
    auto tokenizeResult = tempo_command::tokenize_argv(argc - 1, &argv[1]);
    if (tokenizeResult.isStatus())
        display_status_and_exit(tokenizeResult.getStatus());
    auto tokens = tokenizeResult.getResult();

    // parse remaining options and arguments
    auto status = tempo_command::parse_completely(tokens, groupings, options, arguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"groove-agent"}, "zuri storage agent",
                    {}, groupings, optMappings, argMappings, defaults);
            default:
                return status;
        }
    }

    // convert options to config
    status = convert_options(options, optMappings, config);
    if (!status.isOk())
        return status;

    // convert arguments to config
    status = convert_arguments(arguments, argMappings, config);
    if (!status.isOk())
        return status;

    TU_LOG_INFO << "config:\n" << tempo_command::command_config_to_string(config);

    // determine the agent name
    std::string agentName;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(agentName, agentNameParser,
        config, "agentName"));

    // determine the listener url
    tempo_utils::Url listenerUrl;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(listenerUrl, listenerUrlParser,
        config, "listenerUrl"));

    // determine the pem certificate file
    std::filesystem::path pemCertificateFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(pemCertificateFile, pemCertificateFileParser,
        config, "pemCertificateFile"));

    // determine the pem private key file
    std::filesystem::path pemPrivateKeyFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(pemPrivateKeyFile, pemPrivateKeyFileParser,
        config, "pemPrivateKeyFile"));

    // determine the pem root CA bundle file
    std::filesystem::path pemRootCABundleFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(pemRootCABundleFile, pemRootCABundleFileParser,
        config, "pemRootCABundleFile"));

    // parse the run in background flag
    bool runInBackground;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(runInBackground, runInBackgroundParser,
        config, "runInBackground"));

    // parse the temporary session flag
    bool temporarySession;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(temporarySession, temporarySessionParser,
        config, "temporarySession"));

    // parse the transport type
    std::string transport;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(transport, transportParser,
        config, "transport"));

    // parse the idle timeout option
    int idleTimeout;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(idleTimeout, idleTimeoutParser,
        config, "idleTimeout"));

    // parse the emit endpoint flag
    bool emitEndpoint;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(emitEndpoint, emitEndpointParser,
        config, "emitEndpoint"));

    // determine the log file
    std::filesystem::path logFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(logFile, logFileParser,
        config, "logFile"));

    // determine the pid file
    std::filesystem::path pidFile;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(pidFile, pidFileParser,
        config, "pidFile"));

    // parse the list of dataset configs
    std::vector<tempo_config::ConfigNode> datasetConfigList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(datasetConfigList, datasetDataListParser,
        config, "datasetDataList"));

    // parse the list of dataset config files
    std::vector<tempo_config::ConfigFile> datasetFileList;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(datasetFileList, datasetFileListParser,
        config, "datasetFileList"));

    // construct the map of models from the combination of input config and model configs
    absl::flat_hash_map<tempo_utils::Url,DatasetConfig> datasetConfigs;

    // parse each model config file argument
    for (const auto &configFile : datasetFileList) {
        auto rootNode = configFile.getRoot();
        if (rootNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "invalid dataset config file {}", configFile.getPath().string());
        auto configMap = rootNode.toMap();

        tempo_config::UrlParser datasetUrlParser;
        tempo_utils::Url datasetUrl;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(datasetUrl, datasetUrlParser,
            configMap, "datasetUrl"));
        if (datasetConfigs.contains(datasetUrl))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "dataset '{}' is already defined", datasetUrl.toString());

        const auto &dataRoot = configFile.getPath().parent_path();
        status = update_dataset_configs(datasetConfigs, datasetUrl, configMap, dataRoot);
        if (status.notOk())
            return status;
    }

    // parse each model config data argument
    for (const auto &rootNode : datasetConfigList) {
        if (rootNode.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "invalid dataset config data");
        auto configMap = rootNode.toMap();

        tempo_config::UrlParser datasetUrlParser;
        tempo_utils::Url datasetUrl;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(datasetUrl, datasetUrlParser,
            configMap, "datasetUrl"));
        if (datasetConfigs.contains(datasetUrl))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "dataset '{}' is already defined", datasetUrl.toString());

        status = update_dataset_configs(datasetConfigs, datasetUrl, configMap, std::filesystem::current_path());
        if (status.notOk())
            return status;
    }

    // initialize logging
    tempo_utils::LoggingConfiguration loggingConfig;
    loggingConfig.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
    loggingConfig.flushEveryMessage = true;
    if (!logFile.empty()) {
        auto *logFp = std::fopen(logFile.c_str(), "a");
        if (logFp == nullptr) {
            return tempo_utils::PosixStatus::last(
                absl::StrCat("failed to open logfile ", logFile.c_str()));
        }
        loggingConfig.logFile = logFp;
    }
    tempo_utils::init_logging(loggingConfig);

    // generate the listener url if one was not specified
    if (!listenerUrl.isValid()) {
        if (transport == "unix") {
            listenerUrl = tempo_utils::Url::fromString(absl::StrCat("unix://",
                absolute(std::filesystem::current_path().append("node.sock")).string()));
        } else {
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "unknown transport type '{}'", transport);
        }
    }

    // print the agent endpoint if requested
    if (emitEndpoint) {
        TU_CONSOLE_OUT << listenerUrl.toString();
    }

    // if agent should run in the background, then fork and continue in the child
    if (runInBackground) {
        auto pid = fork();
        if (pid < 0)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "failed to fork into the background");
        if (pid > 0) {
            TU_LOG_INFO << "forked node into the background with pid " << pid;
            _exit(0);
        }
        auto sid = setsid();
        if (sid < 0)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "failed to set session");
        TU_LOG_INFO << "set session sid " << sid;
    }

    // TODO: if pid file is specified, then write the pid file

    // initialize uv loop
    uv_loop_t loop;
    uv_loop_init(&loop);

    // configure the local database
    groove_model::DatabaseOptions databaseOptions;
    groove_model::GrooveDatabase db(databaseOptions);
    auto dbConfigureStatus = db.configure();
    if (dbConfigureStatus.notOk())
        return dbConfigureStatus;

    StorageSupervisor supervisor(&db);

    // add all specified datasets to the store
    for (const auto &datasetEntry : datasetConfigs) {
        const auto &datasetUrl = datasetEntry.first;
        const auto &datasetConfig = datasetEntry.second;

        std::shared_ptr<StorageCollection> store;
        auto getOrCreateResult = supervisor.getOrCreateCollection(datasetConfig.collectionName);
        if (getOrCreateResult.isStatus())
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "failed to get or create collection: {}", getOrCreateResult.getStatus().getMessage());
        auto result = getOrCreateResult.getResult();
        store = result.first;

        auto linkDatasetStatus = store->linkDatasetFile(datasetUrl, datasetConfig.dataPath);
        if (linkDatasetStatus.notOk())
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "failed to link dataset {}: {}",
                datasetConfig.dataPath.string(), linkDatasetStatus.getMessage());
    }

    // enable the channelz service which lets us monitor active connections
    grpc::channelz::experimental::InitChannelzService();

    // construct the storage and syncing services
    MountService mountService(listenerUrl, &supervisor, agentName);
    SyncService syncingService(listenerUrl, &supervisor, agentName);

    // construct the server and start it up
    grpc::ServerBuilder builder;
    auto credentials = make_ssl_server_credentials(pemCertificateFile,
        pemPrivateKeyFile, pemRootCABundleFile);
    builder.AddListeningPort(listenerUrl.toString(), credentials);
    builder.AddChannelArgument(GRPC_ARG_ENABLE_CHANNELZ, 1);
    builder.RegisterService(&mountService);
    builder.RegisterService(&syncingService);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    TU_LOG_INFO << "starting service using endpoint " << listenerUrl.toString();

    // construct the channelz client stub
    grpc::ChannelArguments args;
    auto channelz = grpc::channelz::v1::Channelz::NewStub(server->InProcessChannel(args));
    ChannelMonitor monitor(&loop, channelz.get(), 5);
    monitor.initialize();

    // catch SIGTERM indicating request to cleanly shutdown
    uv_signal_t sigterm;
    uv_signal_init(&loop, &sigterm);
    sigterm.data = server.get();
    uv_signal_start_oneshot(&sigterm, on_termination_signal, SIGTERM);

    // catch SIGINT indicating request to cleanly shutdown
    uv_signal_t sigint;
    uv_signal_init(&loop, &sigint);
    sigint.data = server.get();
    uv_signal_start_oneshot(&sigint, on_termination_signal, SIGINT);

    // redirect stdout to null
    int nullfd = open("/dev/null", O_WRONLY | O_EXCL | O_CLOEXEC);
    if (nullfd < 0)
        return tempo_utils::PosixStatus::last("failed to open /dev/null");
    dup2(nullfd, STDOUT_FILENO);
    close(nullfd);

    // run main loop waiting for a termination signal
    TU_LOG_INFO << "entering main loop";
    auto ret = uv_run(&loop, UV_RUN_DEFAULT);
    TU_LOG_INFO << "exiting main loop";
    if (ret < 0)
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to run main loop");

    uv_close((uv_handle_t *) &sigterm, nullptr);
    uv_close((uv_handle_t *) &sigint, nullptr);

    //
    for (;;) {
        ret = uv_loop_close(&loop);
        if (ret == 0)
            break;
        if (ret != UV_EBUSY)
            break;
        uv_run(&loop, UV_RUN_NOWAIT);
    }
    TU_LOG_INFO << "closed main loop";

    //
    server->Shutdown();

    return tempo_command::CommandStatus::ok();
}