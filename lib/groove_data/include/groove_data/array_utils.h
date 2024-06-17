#ifndef GROOVE_DATA_ARRAY_UTILS_H
#define GROOVE_DATA_ARRAY_UTILS_H

#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <tempo_utils/integer_types.h>
#include <tempo_utils/log_stream.h>

#include "data_types.h"

namespace groove_data {

    inline
    bool get_double_datum(
        std::shared_ptr<arrow::ChunkedArray> chunkedArray,
        int index,
        double &value)
    {
        for (const auto &chunk : chunkedArray->chunks()) {
            auto array = std::static_pointer_cast<arrow::DoubleArray>(chunk);
            if (index < array->length()) {
                value = array->Value(index);
                return true;
            }
            index -= array->length();
        }
        return false;
    };

    inline
    bool get_int64_datum(
        std::shared_ptr<arrow::ChunkedArray> chunkedArray,
        int index,
        tu_int64 &value)
    {
        for (const auto &chunk : chunkedArray->chunks()) {
            auto array = std::static_pointer_cast<arrow::Int64Array>(chunk);
            if (index < array->length()) {
                value = array->Value(index);
                return true;
            }
            index -= array->length();
        }
        return false;
    };

    inline
    bool get_string_datum(
        std::shared_ptr<arrow::ChunkedArray> chunkedArray,
        int index,
        std::string &value)
    {
        for (const auto &chunk : chunkedArray->chunks()) {
            auto array = std::static_pointer_cast<arrow::StringArray>(chunk);
            if (index < array->length()) {
                value = array->Value(index);
                return true;
            }
            index -= array->length();
        }
        return false;
    };

    inline
    bool get_category_datum(
        std::shared_ptr<arrow::ChunkedArray> chunkedArray,
        int index,
        Category &value)
    {
        for (const auto &chunk : chunkedArray->chunks()) {
            auto larray = std::static_pointer_cast<arrow::ListArray>(chunk);
            if (index < larray->length()) {
                auto sarray = std::static_pointer_cast<arrow::StringArray>(larray->values());
                auto offset = larray->value_offset(index);
                auto length = larray->value_length(index);
                std::vector<std::string> path;
                for (int i = 0; i < length; i++) {
                    path.push_back(std::string(sarray->Value(offset + i)));
                }
                value = Category(path);
                return true;
            }
            index -= larray->length();
        }
        return false;
    };
}

#endif // GROOVE_DATA_ARRAY_UTILS_H