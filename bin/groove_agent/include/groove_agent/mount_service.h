#ifndef GROOVE_AGENT_MOUNT_SERVICE_H
#define GROOVE_AGENT_MOUNT_SERVICE_H

#include <boost/uuid/random_generator.hpp>

#include <groove_mount/mount_service.grpc.pb.h>
#include <groove_storage/storage_result.h>
#include <tempo_utils/url.h>

#include "storage_collection.h"
#include "storage_supervisor.h"

class CollectionChangeWriter
    : public grpc::ServerWriteReactor<groove_mount::CollectionChange>,
      public AbstractCollectionWatcher
{
public:
    CollectionChangeWriter();
    CollectionChangeWriter(std::shared_ptr<StorageCollection> container);
    ~CollectionChangeWriter();

    tempo_utils::Status attach();
    tempo_utils::Status detach();
    void collectionMounted(bool created);

    void OnWriteDone(bool ok) override;
    void OnDone() override;
    void containerAdded(const tempo_utils::Url &containerUrl) override;
    void containerRemoved(const tempo_utils::Url &containerUrl) override;
    void datasetAdded(const tempo_utils::Url &datasetUrl) override;
    void datasetRemoved(const tempo_utils::Url &datasetUrl) override;

    struct PendingWrite {
        groove_mount::CollectionChange change;
        PendingWrite *next;
    };

private:
    absl::Mutex m_lock;
    std::shared_ptr<StorageCollection> m_collection;
    tu_uint64 m_watchkey;
    PendingWrite *m_head;
    PendingWrite *m_tail;

    void enqueue(PendingWrite *write);
};

class MountService : public groove_mount::MountService::CallbackService {

public:
    MountService(
        const tempo_utils::Url &listenerUrl,
        StorageSupervisor *supervisor,
        std::string_view agentName);

    grpc::ServerUnaryReactor *
    IdentifyAgent(
        grpc::CallbackServerContext *context,
        const groove_mount::IdentifyAgentRequest *request,
        groove_mount::IdentifyAgentResult *response) override;

    grpc::ServerWriteReactor<groove_mount::CollectionChange> *
    MountCollection(
        grpc::CallbackServerContext *context,
        const groove_mount::MountCollectionRequest *request) override;

    grpc::ServerUnaryReactor *
    CreateDataset(
        grpc::CallbackServerContext *context,
        const groove_mount::CreateDatasetRequest *request,
        groove_mount::CreateDatasetResult *response) override;

    grpc::ServerUnaryReactor *
    PutData(
        grpc::CallbackServerContext *context,
        const groove_mount::PutDataRequest *request,
        groove_mount::PutDataResult *response) override;

private:
    tempo_utils::Url m_listenerUrl;
    StorageSupervisor *m_supervisor;
    std::string m_agentName;
    tu_uint64 m_uptime;
    boost::uuids::random_generator m_uuidgen;
};

#endif // GROOVE_AGENT_MOUNT_SERVICE_H