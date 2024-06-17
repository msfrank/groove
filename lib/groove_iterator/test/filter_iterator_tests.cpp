#include <gtest/gtest.h>

#include <groove_iterator/filter_iterator_template.h>
#include <groove_iterator/range_iterator_template.h>

static bool is_even(const tu_int64 &value)
{
    return (value % 2) == 0;
}

TEST(FilterIteratorTest, TestMatchesFilter)
{
    auto range = std::make_shared<std::vector<tu_int64>>();
    range->push_back(1);
    range->push_back(2);
    range->push_back(3);
    groove_iterator::RangeIterator<std::vector<tu_int64>> it(range, range->cbegin(), range->cend());
    groove_iterator::FilterIterator<
        groove_iterator::RangeIterator<
            std::vector<tu_int64>>,tu_int64> filter(it, is_even);

    tu_int64 value;
    ASSERT_TRUE (filter.getNext(value));
    ASSERT_EQ (2, value);
    ASSERT_FALSE (filter.getNext(value));
}

TEST(FilterIteratorTest, TestNoMatches)
{
    auto range = std::make_shared<std::vector<tu_int64>>();
    range->push_back(1);
    range->push_back(3);
    range->push_back(5);
    groove_iterator::RangeIterator<std::vector<tu_int64>> it(range, range->cbegin(), range->cend());
    groove_iterator::FilterIterator<
        groove_iterator::RangeIterator<
            std::vector<tu_int64>>,tu_int64> filter(it, is_even);

    tu_int64 value;
    ASSERT_FALSE (filter.getNext(value));
}

TEST(FilterIteratorTest, TestEmpty)
{
    auto range = std::make_shared<std::vector<tu_int64>>();
    groove_iterator::RangeIterator<std::vector<tu_int64>> it(range, range->cbegin(), range->cend());
    groove_iterator::FilterIterator<
        groove_iterator::RangeIterator<
            std::vector<tu_int64>>,tu_int64> filter(it, is_even);

    tu_int64 value;
    ASSERT_FALSE (filter.getNext(value));
}