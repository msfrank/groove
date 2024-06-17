#ifndef GROOVE_MODEL_BASE_PAGE_H
#define GROOVE_MODEL_BASE_PAGE_H

#include <arrow/buffer.h>

#include "model_result.h"
#include "model_types.h"
#include "page_id.h"

namespace groove_model {

    class BasePage {

    public:
        virtual ~BasePage() = default;

        PageId getPageId() const;
        std::shared_ptr<const std::string> getColumnId() const;

    protected:
        BasePage(PageId pageId, std::shared_ptr<const std::string> columnId);

    private:
        PageId m_pageId;
        std::shared_ptr<const std::string> m_columnId;
        std::shared_ptr<const std::string> m_bytes;
    };
}

#endif // GROOVE_MODEL_BASE_PAGE_H