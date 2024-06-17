
#include <groove_data/comparison_utils.h>

int
groove_data::compare_key(const groove_data::Category &lhs, const groove_data::Category &rhs)
{
    return lhs.compare(rhs);
}

int
groove_data::compare_key(double lhs, double rhs)
{
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

int
groove_data::compare_key(tu_int64 lhs, tu_int64 rhs)
{
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

int
groove_data::compare_value(double lhs, double rhs)
{
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

int
groove_data::compare_value(tu_int64 lhs, tu_int64 rhs)
{
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

int
groove_data::compare_value(const std::string &lhs, const std::string &rhs)
{
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

bool
groove_data::cmp_sorted_int64_int64_datum(
    const groove_data::Int64Int64Datum &lhs,
    const groove_data::Int64Int64Datum &rhs)
{
    if (lhs.key < rhs.key)
        return true;
    if (lhs.key > rhs.key)
        return false;
    return (lhs.key == rhs.key && lhs.value < rhs.value);
}

bool
groove_data::cmp_indexed_int64_int64_datum(
    const groove_data::Int64Int64Datum &lhs,
    const groove_data::Int64Int64Datum &rhs)
{
    return (lhs.key < rhs.key);
}

bool
groove_data::equals_sorted_int64_int64_datum(
    const groove_data::Int64Int64Datum &lhs,
    const groove_data::Int64Int64Datum &rhs)
{
    return (lhs.key == rhs.key) && (lhs.value == rhs.value);
}

bool
groove_data::equals_indexed_int64_int64_datum(
    const groove_data::Int64Int64Datum &lhs,
    const groove_data::Int64Int64Datum &rhs)
{
    return lhs.key == rhs.key;
}

bool
groove_data::cmp_sorted_int64_double_datum(
    const groove_data::Int64DoubleDatum &lhs,
    const groove_data::Int64DoubleDatum &rhs)
{
    if (lhs.key < rhs.key)
        return true;
    if (lhs.key > rhs.key)
        return false;
    return (lhs.key == rhs.key && lhs.value < rhs.value);
}

bool
groove_data::cmp_indexed_int64_double_datum(
    const groove_data::Int64DoubleDatum &lhs,
    const groove_data::Int64DoubleDatum &rhs)
{
    return (lhs.key < rhs.key);
}

bool
groove_data::equals_sorted_int64_double_datum(
    const groove_data::Int64DoubleDatum &lhs,
    const groove_data::Int64DoubleDatum &rhs)
{
    return (lhs.key == rhs.key && lhs.value == rhs.value);
}

bool
groove_data::equals_indexed_int64_double_datum(
    const groove_data::Int64DoubleDatum &lhs,
    const groove_data::Int64DoubleDatum &rhs)
{
    return (lhs.key == rhs.key);
}

bool
groove_data::cmp_sorted_int64_string_datum(
    const groove_data::Int64StringDatum &lhs,
    const groove_data::Int64StringDatum &rhs)
{
    if (lhs.key < rhs.key)
        return true;
    if (lhs.key > rhs.key)
        return false;
    return (lhs.key == rhs.key && lhs.value < rhs.value);
}

bool
groove_data::cmp_indexed_int64_string_datum(
    const groove_data::Int64StringDatum &lhs,
    const groove_data::Int64StringDatum &rhs)
{
    return (lhs.key < rhs.key);
}

bool
groove_data::equals_sorted_int64_string_datum(
    const groove_data::Int64StringDatum &lhs,
    const groove_data::Int64StringDatum &rhs)
{
    return (lhs.key == rhs.key && lhs.value == rhs.value);
}

bool
groove_data::equals_indexed_int64_string_datum(
    const groove_data::Int64StringDatum &lhs,
    const groove_data::Int64StringDatum &rhs)
{
    return (lhs.key == rhs.key);
}
