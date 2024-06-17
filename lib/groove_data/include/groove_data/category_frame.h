#ifndef GROOVE_DATA_CATEGORY_FRAME_H
#define GROOVE_DATA_CATEGORY_FRAME_H

#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_result.h"
#include "data_types.h"

namespace groove_data {

    class CategoryFrame : public BaseFrame, public std::enable_shared_from_this<CategoryFrame> {
    public:
        DataFrameType getFrameType() const override;
        DataKeyType getKeyType() const override;
        int getKeyFieldIndex() const override;

        Option<Category> smallestKey() const;
        Option<Category> largestKey() const;

        static tempo_utils::Result<std::shared_ptr<CategoryFrame>>
        create(
            std::shared_ptr<arrow::Table> table,
            int keyFieldIndex,
            const std::vector<std::pair<int,int>> &valueColumns);

    private:
        int m_keyFieldIndex;

        CategoryFrame(std::shared_ptr<arrow::Table> table, int keyColumn);
    };
}

#endif // GROOVE_DATA_CATEGORY_FRAME_H