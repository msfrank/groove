#ifndef GROOVE_STORAGE_GROOVE_STORAGE_H
#define GROOVE_STORAGE_GROOVE_STORAGE_H

#include <filesystem>
#include <vector>

#include <grpcpp/channel.h>
#include <grpcpp/security/credentials.h>

#include <tempo_config/config_types.h>
#include <tempo_utils/url.h>

#include "remote_collection.h"
#include "storage_client.h"
#include "storage_result.h"
#include "storage_types.h"

namespace groove_storage {

    enum class EndpointTransport {
        UNIX,
        TCP,
    };

    enum class AgentDiscoveryPolicy {
        USE_SPECIFIED_ENDPOINT,
        SPAWN_IF_MISSING,
        ALWAYS_SPAWN,
    };

    struct StorageOptions {
        AgentDiscoveryPolicy discoveryPolicy;
        tempo_utils::Url agentEndpoint;
        EndpointTransport endpointTransport;
        std::string agentServerName;
        std::filesystem::path agentPath;
        std::filesystem::path runDirectory;
        std::filesystem::path pemCertificateFile;
        std::filesystem::path pemPrivateKeyFile;
        std::filesystem::path pemRootCABundleFile;
    };

    struct StoragePriv;

    class GrooveStorage {

    public:
        GrooveStorage();
        explicit GrooveStorage(const StorageOptions &options = {});
        ~GrooveStorage();

        tempo_utils::Status initialize();

        tempo_utils::Url getAgentEndpoint() const;
        std::shared_ptr<grpc::ChannelCredentials> getCredentials() const;
        std::string getAgentServerName() const;

        tempo_utils::Result<std::shared_ptr<RemoteCollection>> mountCollection(
            std::string_view name,
            const tempo_config::ConfigMap &configMap,
            MountMode mode);

        StorageStatus shutdown();

    private:
        StorageOptions m_options;
        tempo_utils::Url m_agentEndpoint;
        std::shared_ptr<grpc::ChannelCredentials> m_credentials;
        std::string m_agentServerName;
        std::shared_ptr<StorageClient> m_client;
    };
}

#endif // GROOVE_STORAGE_GROOVE_STORAGE_H