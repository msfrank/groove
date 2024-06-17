//#ifndef GROOVE_MODEL_PERSISTENT_CACHING_PAGE_STORE_H
//#define GROOVE_MODEL_PERSISTENT_CACHING_PAGE_STORE_H
//
//#include <tempo_utils/option_template.h>
//
//#include "indexed_page_template.h"
//#include "model_result.h"
//#include "model_types.h"
//#include "page_id.h"
//#include "rocksdb_store.h"
//
//namespace groove_model {
//
//    class PersistentCachingPageStore {
//
//    public:
//        PersistentCachingPageStore(std::shared_ptr<RocksDbStore> store);
//
//        std::shared_ptr<RocksDbStore> getStore() const;
//
//    private:
//        std::shared_ptr<RocksDbStore> m_store;
//
//    public:
//
//        /**
//         *
//         * @tparam DefType
//         * @tparam KeyType
//         * @param modelId
//         * @param columnId
//         * @param key
//         * @param exclusive
//         * @return
//         */
//        template <typename DefType,
//            typename KeyType = typename DefType::KeyType>
//        ModelResult<std::shared_ptr<IndexedPage<DefType>>>
//        getIndexedPage(
//            const tempo_utils::URI &datasetUrl,
//            std::shared_ptr<const std::string> modelId,
//            std::shared_ptr<const std::string> columnId,
//            const Option<KeyType> &key,
//            bool exclusive)
//        {
//            auto pageId = PageId::create<DefType,groove_data::CollationMode::COLLATION_INDEXED>(
//                datasetUrl, modelId, columnId, key);
//
//            // create the necessary search keys
//            std::string searchKey = pageId.getBytes();
//            std::string searchPrefix = pageId.getPrefix();
//
//            // TODO: check the page cache first
//
//            std::shared_ptr<const std::string> keyFound;
//            rocksdb::Status status;
//
//            // check the persistent model for a possible page containing the item
//            auto store = getStore();
//            if (!exclusive) {
//                keyFound = store->getKeyBefore(&status, searchKey, searchPrefix, false);
//            } else {
//                keyFound = store->getKeyAfter(&status, searchKey, searchPrefix, true);
//            }
//            if (!status.ok())
//                return ModelStatus::internalViolation(status.ToString());
//            if (!keyFound)
//                return ModelStatus::pageNotFound(searchKey);
//
//            // if we found a page, then fetch the page contents
//            if (!keyFound->empty()) {
//                auto bytes = store->getValue(&status, *keyFound);
//                if (!status.ok())
//                    return ModelStatus::internalViolation(status.ToString());
//                if (!bytes || bytes->empty())
//                    return ModelStatus::internalViolation("invalid page");
//                auto idFound = PageId::fromString(*keyFound);
//                auto pageFound = IndexedPage<DefType>::fromBytes(idFound, bytes);
//                // TODO: write back page to cache
//                return pageFound;
//            }
//
//            return ModelStatus::pageNotFound(searchKey);
//        };
//    };
//}
//
//#endif // GROOVE_MODEL_PERSISTENT_CACHING_PAGE_STORE_H