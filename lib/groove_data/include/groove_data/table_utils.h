#ifndef GROOVE_DATA_TABLE_UTILS_H
#define GROOVE_DATA_TABLE_UTILS_H

#include <string>

#include <arrow/buffer.h>
#include <arrow/table.h>

#include "data_result.h"

namespace groove_data {

    tempo_utils::Result<std::shared_ptr<arrow::Table>> make_table(std::shared_ptr<arrow::Buffer> buffer);

    tempo_utils::Result<std::shared_ptr<arrow::Table>> make_table(std::shared_ptr<const std::string> bytes);

    tempo_utils::Result<std::shared_ptr<const arrow::Buffer>> make_buffer(std::shared_ptr<const arrow::Table> table);
}

#endif // GROOVE_DATA_TABLE_UTILS_H