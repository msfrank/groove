#include <gtest/gtest.h>

#include <groove_iterator/map_iterator_template.h>
#include <groove_iterator/range_iterator_template.h>

static std::string int64_to_string(const tu_int64 &value)
{
    return absl::StrCat(value);
}

TEST(MapIteratorTest, TestMapNonEmpty)
{
    auto range = std::make_shared<std::vector<tu_int64>>();
    range->push_back(1);
    range->push_back(2);
    range->push_back(3);
    groove_iterator::RangeIterator<std::vector<tu_int64>> src(range, range->cbegin(), range->cend());
    groove_iterator::MapIterator<
        groove_iterator::RangeIterator<
            std::vector<tu_int64>>,tu_int64,std::string> it(src, int64_to_string);

    std::string value;
    ASSERT_TRUE (it.getNext(value));
    ASSERT_EQ ("1", value);
    ASSERT_TRUE (it.getNext(value));
    ASSERT_EQ ("2", value);
    ASSERT_TRUE (it.getNext(value));
    ASSERT_EQ ("3", value);
    ASSERT_FALSE (it.getNext(value));
}

TEST(MapIteratorTest, TestMapEmpty)
{
    auto range = std::make_shared<std::vector<tu_int64>>();
    groove_iterator::RangeIterator<std::vector<tu_int64>> src(range, range->cbegin(), range->cend());
    groove_iterator::MapIterator<
        groove_iterator::RangeIterator<
            std::vector<tu_int64>>,tu_int64,std::string> it(src, int64_to_string);

    std::string value;
    ASSERT_FALSE (it.getNext(value));
}