#include <sys/wait.h>

#include <absl/container/flat_hash_map.h>
#include <absl/strings/ascii.h>

#include <groove_storage/groove_storage.h>
#include <groove_mount/mount_service.grpc.pb.h>
#include <tempo_config/config_serde.h>
#include <tempo_utils/daemon_process.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/process_utils.h>
#include <tempo_utils/url.h>

namespace groove_storage {
    struct StoragePriv {
        std::unique_ptr<groove_mount::MountService::Stub> stub;
    };
}

groove_storage::GrooveStorage::GrooveStorage()
{
}

groove_storage::GrooveStorage::GrooveStorage(const StorageOptions &options)
    : m_options(options)
{
}

groove_storage::GrooveStorage::~GrooveStorage()
{
}

struct AgentParams {
    std::shared_ptr<tempo_utils::DaemonProcess> agentProcess;
    tempo_utils::Url agentEndpoint;
    std::string agentServerName;
    std::filesystem::path pemRootCABundleFile;
};

static tempo_utils::Result<AgentParams>
spawn_temporary_agent(
    const std::filesystem::path &agentPath,
    groove_storage::EndpointTransport transport,
    const std::string agentServerName,
    const std::filesystem::path &runDirectory,
    const std::filesystem::path &pemCertificateFile,
    const std::filesystem::path &pemPrivateKeyFile,
    const std::filesystem::path &pemRootCABundleFile)
{
    std::string transportType;
    switch (transport) {
        case groove_storage::EndpointTransport::UNIX:
            transportType = "unix";
            break;
        case groove_storage::EndpointTransport::TCP:
            transportType = "tcp";
            break;
        default:
            return groove_storage::StorageStatus::forCondition(
                groove_storage::StorageCondition::kStorageInvariant, "invalid transport");
    }

    tempo_utils::ProcessBuilder builder(agentPath);
    builder.appendArg("--temporary-session");
    builder.appendArg("--transport", transportType);
    builder.appendArg("--emit-endpoint");
    builder.appendArg("--idle-timeout", "30");
    builder.appendArg("--background");
    builder.appendArg("--certificate", pemCertificateFile.string());
    builder.appendArg("--private-key", pemPrivateKeyFile.string());
    builder.appendArg("--ca-bundle", pemRootCABundleFile.string());
    auto invoker = builder.toInvoker();

    auto agentProcess = tempo_utils::DaemonProcess::spawn(invoker, runDirectory);
    if (agentProcess == nullptr)
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "failed to spawn agent");

    auto agentEndpointString = agentProcess->getChildOutput();
    absl::StripAsciiWhitespace(&agentEndpointString);
    if (agentEndpointString.empty())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "missing agent endpoint");
    auto agentEndpoint = tempo_utils::Url::fromString(agentEndpointString);
    if (!agentEndpoint.isValid())
        return groove_storage::StorageStatus::forCondition(
            groove_storage::StorageCondition::kStorageInvariant, "invalid agent endpoint");
    TU_LOG_INFO << "agent endpoint is " << agentEndpoint;


    // return the agent params
    AgentParams params;
    params.agentProcess = agentProcess;
    params.agentEndpoint = agentEndpoint;
    params.agentServerName = agentServerName;
    params.pemRootCABundleFile = pemRootCABundleFile;
    return params;
}

static tempo_utils::Result<AgentParams>
connect_to_specified_endpoint(
    const tempo_utils::Url &agentEndpoint,
    const std::string agentServerName,
    const std::filesystem::path &pemRootCABundleFile)
{
    return groove_storage::StorageStatus::forCondition(
        groove_storage::StorageCondition::kStorageInvariant, "connect_to_specified_endpoint is unimplemented");
}

static tempo_utils::Result<AgentParams>
spawn_temporary_agent_if_missing(
    const tempo_utils::Url &agentEndpoint,
    const std::filesystem::path &agentPath,
    groove_storage::EndpointTransport transport,
    const std::string agentServerName,
    const std::filesystem::path &runDirectory,
    const std::filesystem::path &pemCertificateFile,
    const std::filesystem::path &pemPrivateKeyFile,
    const std::filesystem::path &pemRootCABundleFile)
{
    return groove_storage::StorageStatus::forCondition(
        groove_storage::StorageCondition::kStorageInvariant, "spawn_temporary_agent_if_missing is unimplemented");
}

tempo_utils::Status
groove_storage::GrooveStorage::initialize()
{
    if (m_client != nullptr)
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant, "storage is already initialized");

    // spawn new agent or connect to existing agent, depending on policy
    AgentParams params;
    switch (m_options.discoveryPolicy) {
        case AgentDiscoveryPolicy::USE_SPECIFIED_ENDPOINT: {
            auto useSpecifiedEndpointResult = connect_to_specified_endpoint(
                m_options.agentEndpoint,
                m_options.agentServerName,
                m_options.pemRootCABundleFile);
            if (useSpecifiedEndpointResult.isStatus())
                return useSpecifiedEndpointResult.getStatus();
            params = useSpecifiedEndpointResult.getResult();
            break;
        }
        case AgentDiscoveryPolicy::SPAWN_IF_MISSING: {
            auto spawnIfMissingResult = spawn_temporary_agent_if_missing(
                m_options.agentEndpoint,
                m_options.agentPath,
                m_options.endpointTransport,
                m_options.agentServerName,
                m_options.runDirectory,
                m_options.pemCertificateFile,
                m_options.pemPrivateKeyFile,
                m_options.pemRootCABundleFile);
            if (spawnIfMissingResult.isStatus())
                return spawnIfMissingResult.getStatus();
            params = spawnIfMissingResult.getResult();
            break;
        }
        case AgentDiscoveryPolicy::ALWAYS_SPAWN: {
            auto spawnTemporaryAgentResult = spawn_temporary_agent(
                m_options.agentPath,
                m_options.endpointTransport,
                m_options.agentServerName,
                m_options.runDirectory,
                m_options.pemCertificateFile,
                m_options.pemPrivateKeyFile,
                m_options.pemRootCABundleFile);
            if (spawnTemporaryAgentResult.isStatus())
                return spawnTemporaryAgentResult.getStatus();
            params = spawnTemporaryAgentResult.getResult();
            break;
        }
    }

    // construct the channel credentials
    grpc::SslCredentialsOptions options;
    tempo_utils::FileReader rootCABundleReader(params.pemRootCABundleFile);
    if (rootCABundleReader.getStatus().notOk())
        return StorageStatus::forCondition(StorageCondition::kStorageInvariant,
            "failed to read root CA bundle");
    auto rootCABytes = rootCABundleReader.getBytes();
    options.pem_root_certs = std::string((const char *) rootCABytes->getData(), rootCABytes->getSize());
    auto credentials = grpc::SslCredentials(options);

    // construct the client
    auto createClientResult = StorageClient::create(params.agentEndpoint, credentials, params.agentServerName);
    if (createClientResult.isStatus())
        return createClientResult.getStatus();
    m_client = createClientResult.getResult();

    // connect to the remote agent
    auto connectStatus = m_client->connect();
    if (connectStatus.notOk()) {
        TU_LOG_ERROR << "connect failed: " << connectStatus;
        return connectStatus;
    }

    // verify that the agent is running
    auto identifyResult = m_client->identify();
    if (identifyResult.isStatus()) {
        TU_LOG_ERROR << "Identify failed: " << identifyResult.getStatus();
        return identifyResult.getStatus();
    }

    auto identity = identifyResult.getResult();
    TU_LOG_INFO << "connected to agent " << identity.agentName;

    m_agentEndpoint = params.agentEndpoint;
    m_credentials = credentials;
    m_agentServerName = params.agentServerName;

    return StorageStatus::ok();
}

tempo_utils::Url
groove_storage::GrooveStorage::getAgentEndpoint() const
{
    return m_agentEndpoint;
}

std::shared_ptr<grpc::ChannelCredentials>
groove_storage::GrooveStorage::getCredentials() const
{
    return m_credentials;
}

std::string
groove_storage::GrooveStorage::getAgentServerName() const
{
    return m_agentServerName;
}

tempo_utils::Result<std::shared_ptr<groove_storage::RemoteCollection>>
groove_storage::GrooveStorage::mountCollection(
    std::string_view name,
    const tempo_config::ConfigMap &configMap,
    MountMode mode)
{
    auto declareCollectionResult = m_client->declareCollection(std::string(name), configMap, mode);
    if (declareCollectionResult.isStatus())
        return declareCollectionResult.getStatus();
    auto lsn = declareCollectionResult.getResult();
    TU_LOG_INFO << "declared collection " << std::string(name) << " with lsn " << lsn;

    auto storage = std::make_shared<RemoteCollection>(m_client, lsn);
    auto status = m_client->mountCollection(lsn, storage);
    if (status.notOk())
        return status;

    return storage;
}

groove_storage::StorageStatus
groove_storage::GrooveStorage::shutdown()
{
    if (m_client == nullptr)
        return StorageStatus::forCondition(
            StorageCondition::kStorageInvariant, "sandbox is not initialized");
    m_client.reset();
    return StorageStatus::ok();
}
