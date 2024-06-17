#ifndef GROOVE_AGENT_STORAGE_COLLECTION_H
#define GROOVE_AGENT_STORAGE_COLLECTION_H

#include <absl/container/flat_hash_map.h>

#include <groove_io/dataset_file.h>
#include <groove_storage/storage_result.h>
#include <tempo_utils/url.h>
#include "groove_model/groove_database.h"

class AbstractCollectionWatcher {
public:
    virtual ~AbstractCollectionWatcher() = default;
    virtual void containerAdded(const tempo_utils::Url &containerUrl) = 0;
    virtual void containerRemoved(const tempo_utils::Url &containerUrl) = 0;
    virtual void datasetAdded(const tempo_utils::Url &datasetUrl) = 0;
    virtual void datasetRemoved(const tempo_utils::Url &datasetUrl) = 0;
};

class StorageCollection {

public:
    StorageCollection(
        const tempo_utils::Url &collectionUrl,
        groove_model::GrooveDatabase *db,
        int maxWatchers);
    ~StorageCollection();

    tempo_utils::Url getCollectionUrl() const;

    bool hasDataset(const tempo_utils::Url &datasetUrl) const;
    std::shared_ptr<groove_model::AbstractDataset> getDataset(const tempo_utils::Url &datasetUrl) const;

    tempo_utils::Status declareDataset(
        const tempo_utils::Url &datasetUrl,
        groove_model::GrooveSchema schema);
    tempo_utils::Status loadDatasetFile(
        const tempo_utils::Url &datasetUrl,
        std::shared_ptr<groove_io::DatasetFile> datasetFile);
    tempo_utils::Status linkDatasetFile(
        const tempo_utils::Url &datasetUrl,
        const std::filesystem::path &path);

    tempo_utils::Result<bool> removeDataset(const tempo_utils::Url &datasetUrl);

    tempo_utils::Result<tu_uint64> attachWatcher(AbstractCollectionWatcher *watcher);
    tempo_utils::Status detachWatcher(tu_uint64 watchKey);

    tempo_utils::Status drop();

private:
    tempo_utils::Url m_collectionUrl;
    groove_model::GrooveDatabase *m_db;
    int m_maxWatchers;

    absl::Mutex *m_lock;
    absl::flat_hash_map<
        tempo_utils::Url,
        std::shared_ptr<groove_model::AbstractDataset>> m_datasets ABSL_GUARDED_BY(m_lock);
    absl::flat_hash_map<
        tu_uint64,
        AbstractCollectionWatcher *> m_watchers ABSL_GUARDED_BY(m_lock);
    tu_uint64 m_lastKey;
};

#endif // GROOVE_AGENT_STORAGE_COLLECTION_H