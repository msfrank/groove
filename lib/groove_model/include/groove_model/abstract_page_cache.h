#ifndef GROOVE_MODEL_ABSTRACT_PAGE_CACHE_H
#define GROOVE_MODEL_ABSTRACT_PAGE_CACHE_H

#include <string>

#include <tempo_utils/option_template.h>

#include "indexed_page_template.h"
#include "model_result.h"
#include "model_types.h"
#include "page_id.h"

namespace groove_model {

    class AbstractPageCache {

    public:
        virtual ~AbstractPageCache() = default;

        virtual bool isEmpty() = 0;
        virtual tempo_utils::Result<PageId> getPageIdBefore(const PageId &pageId, bool exclusive) = 0;
        virtual tempo_utils::Result<PageId> getPageIdAfter(const PageId &pageId, bool exclusive) = 0;
        virtual tempo_utils::Result<std::shared_ptr<arrow::Buffer>> getPageData(const PageId &pageId) = 0;
        virtual tempo_utils::Status pageExists(const PageId &pageId) = 0;

    public:

        /**
         *
         * @tparam DefType
         * @tparam KeyType
         * @param modelId
         * @param columnId
         * @param key
         * @param exclusive
         * @return
         */
        template <typename DefType,
            typename KeyType = typename DefType::KeyType>
        tempo_utils::Result<std::shared_ptr<IndexedPage<DefType>>>
        getIndexedPage(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            const Option<KeyType> &key,
            bool exclusive)
        {
            auto searchKey = PageId::create<DefType,groove_data::CollationMode::COLLATION_INDEXED>(
                datasetUrl, modelId, columnId, key);

            // check the underlying cache implementation for a possible page containing the item
            PageId pageId;
            if (!exclusive) {
                auto getKeyResult = getPageIdBefore(searchKey, false);
                if (getKeyResult.isStatus())
                    return getKeyResult.getStatus();
                pageId = getKeyResult.getResult();
            } else {
                auto getKeyResult = getPageIdAfter(searchKey, true);
                if (getKeyResult.isStatus())
                    return getKeyResult.getStatus();
                pageId = getKeyResult.getResult();
            }
            if (!pageId.isValid())
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid page id");

            // if we found a page, then fetch the page contents
            auto getDataResult = getPageData(pageId);
            if (getDataResult.isStatus())
                return getDataResult.getStatus();
            auto pageData = getDataResult.getResult();

            if (!pageData || pageData->size() == 0)
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid page");
            auto indexedPage = IndexedPage<DefType>::fromBuffer(pageId, pageData);
            // TODO: write back page to cache
            return indexedPage;
        };
    };
}

#endif // GROOVE_MODEL_ABSTRACT_PAGE_CACHE_H