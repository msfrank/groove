#ifndef GROOVE_MODEL_ROCKSDB_STORE_H
#define GROOVE_MODEL_ROCKSDB_STORE_H

#include <filesystem>
#include <string>
#include <vector>

#include <rocksdb/db.h>

#include "abstract_page_store.h"

namespace groove_model {

    class RocksDbStore : public AbstractPageStore, public std::enable_shared_from_this<RocksDbStore> {

    public:
        ~RocksDbStore() override;

        std::filesystem::path getDbPath() const;

        rocksdb::Status open();

        bool isEmpty() override;
        tempo_utils::Result<PageId> getPageIdBefore(const PageId &pageId, bool exclusive) override;
        tempo_utils::Result<PageId> getPageIdAfter(const PageId &pageId, bool exclusive) override;
        tempo_utils::Result<std::shared_ptr<arrow::Buffer>> getPageData(const PageId &pageId) override;
        tempo_utils::Status pageExists(const PageId &pageId) override;

        AbstractPageStoreTransaction *startTransaction() override;

        std::shared_ptr<const std::string> getKeyBefore(
            rocksdb::Status *status,
            const std::string &key,
            const std::string &prefix,
            bool exclusive);
        std::shared_ptr<const std::string> getKeyAfter(
            rocksdb::Status *status,
            const std::string &key,
            const std::string &prefix,
            bool exclusive);

        std::shared_ptr<arrow::Buffer> getValue(rocksdb::Status *status, const std::string &key);
        void setValue(
            rocksdb::Status *status,
            const std::string &key,
            std::shared_ptr<const arrow::Buffer> value,
            rocksdb::WriteBatch *batch = nullptr);
        void removeValue(
            rocksdb::Status *status,
            const std::string &key,
            rocksdb::WriteBatch *batch = nullptr);
        int valueCount();

        std::string iterateForward(
            rocksdb::Status *status,
            std::vector<std::string> &values,
            int maxValues,
            const std::string &token = {});

        std::string iteratePrefix(
            rocksdb::Status *status,
            const std::string &prefix,
            std::vector<std::string> &values,
            int maxValues,
            const std::string &token = {});

        std::string iterateBounds(
            rocksdb::Status *status,
            const std::string &start,
            const std::string &end,
            std::vector<std::string> &values,
            int maxValues,
            const std::string &token = {});

        std::string getMeta(rocksdb::Status *status, const std::string &key);
        bool metaExists(rocksdb::Status *status, const std::string &key);

        void setMeta(
            rocksdb::Status *status,
            const std::string &key,
            const std::string &value,
            rocksdb::WriteBatch *batch = nullptr);

        void removeMeta(
            rocksdb::Status *status,
            const std::string &key,
            rocksdb::WriteBatch *batch = nullptr);

        rocksdb::Status applyBatch(rocksdb::WriteBatch *batch);

        static std::shared_ptr<RocksDbStore> create(const std::filesystem::path &dbPath);
        static std::shared_ptr<RocksDbStore> create(
            const std::filesystem::path &dbPath,
            const rocksdb::Options &options);

    private:
        std::filesystem::path m_dbPath;
        rocksdb::Options m_options;
        rocksdb::DB *m_rocksDb;

        explicit RocksDbStore(const std::filesystem::path &dbPath);
        RocksDbStore(const std::filesystem::path &dbPath, const rocksdb::Options &options);
    };

    class RocksdbPageData : public arrow::Buffer {
    public:
        RocksdbPageData(rocksdb::PinnableSlice *slice);
        ~RocksdbPageData();
    private:
        rocksdb::PinnableSlice *m_slice;
    };

    class RocksDbTransaction : public AbstractPageStoreTransaction {
    public:
        RocksDbTransaction(std::shared_ptr<RocksDbStore> rocksDbStore);
        ~RocksDbTransaction();

        tempo_utils::Status removePage(const PageId &pageId) override;
        tempo_utils::Status writePage(const PageId &pageId, std::shared_ptr<const arrow::Buffer> pageBytes) override;
        tempo_utils::Status apply() override;
        tempo_utils::Status abort() override;

    private:
        std::shared_ptr<RocksDbStore> m_store;
        rocksdb::WriteBatch *m_batch;
    };
}

#endif // GROOVE_MODEL_ROCKSDB_STORE_H