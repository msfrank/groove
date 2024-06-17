#ifndef GROOVE_DATA_INT64_FRAME_H
#define GROOVE_DATA_INT64_FRAME_H

#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_result.h"
#include "data_types.h"

namespace groove_data {

    class Int64Frame : public BaseFrame, public std::enable_shared_from_this<Int64Frame> {
    public:
        DataFrameType getFrameType() const override;
        DataKeyType getKeyType() const override;
        int getKeyFieldIndex() const override;

        Option<tu_int64> smallestKey() const;
        Option<tu_int64> largestKey() const;

        static tempo_utils::Result<std::shared_ptr<Int64Frame>>
        create(
            std::shared_ptr<arrow::Table> table,
            int keyFieldIndex,
            const std::vector<std::pair<int,int>> &valueColumns);

    private:
        int m_keyFieldIndex;

        Int64Frame(std::shared_ptr<arrow::Table> table, int keyFieldIndex);
    };
}

#endif // GROOVE_DATA_INT64_FRAME_H