#ifndef GROOVE_MODEL_ABSTRACT_PAGE_STORE_H
#define GROOVE_MODEL_ABSTRACT_PAGE_STORE_H

#include "abstract_page_cache.h"
#include "model_result.h"
#include "page_id.h"

namespace groove_model {

    class AbstractPageStoreTransaction {
    public:
        virtual ~AbstractPageStoreTransaction() = default;
        virtual tempo_utils::Status removePage(const PageId &pageId) = 0;
        virtual tempo_utils::Status writePage(const PageId &pageId, std::shared_ptr<const arrow::Buffer> pageBytes) = 0;
        virtual tempo_utils::Status apply() = 0;
        virtual tempo_utils::Status abort() = 0;
    };

    class AbstractPageStore : public AbstractPageCache {

    public:
        virtual ~AbstractPageStore() = default;

        virtual AbstractPageStoreTransaction *startTransaction() = 0;
    };
}

#endif // GROOVE_MODEL_ABSTRACT_PAGE_STORE_H
