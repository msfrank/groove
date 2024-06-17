#ifndef GROOVE_AGENT_STORAGE_SUPERVISOR_H
#define GROOVE_AGENT_STORAGE_SUPERVISOR_H

#include <absl/container/flat_hash_map.h>
#include <boost/uuid/random_generator.hpp>

#include <groove_model/groove_database.h>

#include "storage_collection.h"

constexpr int kMaxWatchersDefault = 64;

class StorageSupervisor {

public:
    StorageSupervisor(groove_model::GrooveDatabase *db);
    ~StorageSupervisor();

    bool hasCollection(std::string_view name) const;
    bool hasCollection(const tempo_utils::Url &uri) const;
    std::shared_ptr<StorageCollection> getCollection(std::string_view name) const;
    std::shared_ptr<StorageCollection> getCollection(const tempo_utils::Url &uri) const;

    tempo_utils::Result<std::pair<std::shared_ptr<StorageCollection>,bool>> getOrCreateCollection(
        std::string_view name);
    tempo_utils::Result<std::shared_ptr<StorageCollection>> createCollection(
        std::string_view name);
    tempo_utils::Result<std::shared_ptr<StorageCollection>> createEphemeralCollection(
        std::string_view name);

    tempo_utils::Result<bool> deleteCollection(std::string_view name);

    bool hasDataset(const tempo_utils::Url &datasetUrl) const;
    std::shared_ptr<groove_model::AbstractDataset> getDataset(const tempo_utils::Url &datasetUrl) const;

    tempo_utils::Result<std::shared_ptr<groove_model::AbstractDataset>> createDataset(
        const tempo_utils::Url &datasetUrl,
        const groove_model::GrooveSchema &schema);
    tempo_utils::Result<bool> deleteDataset(const tempo_utils::Url &datasetUrl);

    tempo_utils::Result<std::vector<std::string>> putData(
        const tempo_utils::Url &datasetUrl,
        const std::string &modelId,
        std::shared_ptr<groove_data::BaseFrame> frame);

private:
    absl::Mutex *m_lock;
    groove_model::GrooveDatabase *m_db;
    absl::flat_hash_map<
        tempo_utils::Url,
        std::shared_ptr<StorageCollection>> m_collections ABSL_GUARDED_BY(m_lock);
    boost::uuids::random_generator m_uuidgen;
};

#endif // GROOVE_AGENT_STORAGE_SUPERVISOR_H