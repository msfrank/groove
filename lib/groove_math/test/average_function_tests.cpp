#include <gtest/gtest.h>

#include <groove_iterator/range_iterator_template.h>
#include <groove_math/reducer_traits.h>
#include <groove_math/reducer_template.h>

TEST(AverageFunction, Reduce)
{
    auto range = std::make_shared<std::vector<double>>();
    range->insert(range->begin(), {1.0, 2.0, 1.0, 2.0});
    auto input = std::make_shared<groove_iterator::RangeIterator<std::vector<double>>>(
        range, range->cbegin(), range->cend());

    groove_math::Reducer<groove_math::Average> reducer(0.0);
    auto result = reducer.reduce(input);
    ASSERT_TRUE (result.isResult());
    auto avg = result.getResult();
    ASSERT_DOUBLE_EQ(1.5, avg);
}