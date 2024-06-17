#ifndef GROOVE_DATA_DOUBLE_FRAME_H
#define GROOVE_DATA_DOUBLE_FRAME_H

#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_result.h"
#include "data_types.h"

namespace groove_data {

    class DoubleFrame : public BaseFrame, public std::enable_shared_from_this<DoubleFrame> {
    public:
        DataFrameType getFrameType() const override;
        DataKeyType getKeyType() const override;
        int getKeyFieldIndex() const override;

        Option<double> smallestKey() const;
        Option<double> largestKey() const;

        static tempo_utils::Result<std::shared_ptr<DoubleFrame>>
        create(
            std::shared_ptr<arrow::Table> table,
            int keyFieldIndex,
            const std::vector<std::pair<int,int>> &valueColumns);

    private:
        int m_keyFieldIndex;

        DoubleFrame(std::shared_ptr<arrow::Table> table, int keyFieldIndex);
    };
}

#endif // GROOVE_DATA_DOUBLE_FRAME_H