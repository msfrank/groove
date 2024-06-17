#ifndef GROOVE_MODEL_COMPARISON_UTILS_H
#define GROOVE_MODEL_COMPARISON_UTILS_H

#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>

namespace groove_data {

    int compare_key(const groove_data::Category &lhs, const groove_data::Category &rhs);
    int compare_key(double lhs, double rhs);
    int compare_key(tu_int64 lhs, tu_int64 rhs);

    int compare_value(double lhs, double rhs);
    int compare_value(tu_int64 lhs, tu_int64 rhs);
    int compare_value(const std::string &lhs, const std::string &rhs);

    bool cmp_sorted_int64_int64_datum(
        const groove_data::Int64Int64Datum &lhs,
        const groove_data::Int64Int64Datum &rhs);
    bool cmp_indexed_int64_int64_datum(
        const groove_data::Int64Int64Datum &lhs,
        const groove_data::Int64Int64Datum &rhs);
    bool equals_sorted_int64_int64_datum(
        const groove_data::Int64Int64Datum &lhs,
        const groove_data::Int64Int64Datum &rhs);
    bool equals_indexed_int64_int64_datum(
        const groove_data::Int64Int64Datum &lhs,
        const groove_data::Int64Int64Datum &rhs);

    bool cmp_sorted_int64_double_datum(
        const groove_data::Int64DoubleDatum &lhs,
        const groove_data::Int64DoubleDatum &rhs);
    bool cmp_indexed_int64_double_datum(
        const groove_data::Int64DoubleDatum &lhs,
        const groove_data::Int64DoubleDatum &rhs);
    bool equals_sorted_int64_double_datum(
        const groove_data::Int64DoubleDatum &lhs,
        const groove_data::Int64DoubleDatum &rhs);
    bool equals_indexed_int64_double_datum(
        const groove_data::Int64DoubleDatum &lhs,
        const groove_data::Int64DoubleDatum &rhs);

    bool cmp_sorted_int64_string_datum(
        const groove_data::Int64StringDatum &lhs,
        const groove_data::Int64StringDatum &rhs);
    bool cmp_indexed_int64_string_datum(
        const groove_data::Int64StringDatum &lhs,
        const groove_data::Int64StringDatum &rhs);
    bool equals_sorted_int64_string_datum(
        const groove_data::Int64StringDatum &lhs,
        const groove_data::Int64StringDatum &rhs);
    bool equals_indexed_int64_string_datum(
        const groove_data::Int64StringDatum &lhs,
        const groove_data::Int64StringDatum &rhs);
}

#endif // GROOVE_MODEL_COMPARISON_UTILS_H