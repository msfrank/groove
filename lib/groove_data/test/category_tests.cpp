
#include <gtest/gtest.h>

#include <groove_data/category.h>

TEST(Category, CompareEqual)
{
    using namespace groove_data;
    ASSERT_EQ(Category({"a"}), Category({"a"}));
    ASSERT_EQ(Category({"a","a"}), Category({"a","a"}));
    ASSERT_EQ(Category({"a","a","a"}), Category({"a","a","a"}));
}

TEST(Category, CompareLessThan)
{
    using namespace groove_data;
    ASSERT_LT(Category({"a"}), Category({"b"}));
    ASSERT_LT(Category({"a","a"}), Category({"a","b"}));
    ASSERT_LT(Category({"a","b"}), Category({"a","b","c"}));
}

TEST(Category, CompareLessThanEqual)
{
    using namespace groove_data;
    ASSERT_LE(Category({"a"}), Category({"b"}));
    ASSERT_LE(Category({"a","a"}), Category({"a","b"}));
    ASSERT_LE(Category({"a","b"}), Category({"a","b","c"}));
    ASSERT_LE(Category({"a"}), Category({"a"}));
    ASSERT_LE(Category({"a","a"}), Category({"a","a"}));
    ASSERT_LE(Category({"a","a","a"}), Category({"a","a","a"}));
}

TEST(Category, CompareGreaterThan)
{
    using namespace groove_data;
    ASSERT_GT(Category({"b"}), Category({"a"}));
    ASSERT_GT(Category({"b","a"}), Category({"a","a"}));
    ASSERT_GT(Category({"b","a","c"}), Category({"b","a","b"}));
}

TEST(Category, CompareGreaterThanEqual)
{
    using namespace groove_data;
    ASSERT_GE(Category({"a"}), Category({"a"}));
    ASSERT_GE(Category({"a","a"}), Category({"a","a"}));
    ASSERT_GE(Category({"a","a","a"}), Category({"a","a","a"}));
    ASSERT_GE(Category({"b"}), Category({"a"}));
    ASSERT_GE(Category({"b","a"}), Category({"a","a"}));
}

TEST(Category, Compare)
{
    using namespace groove_data;
    ASSERT_EQ(0, Category({"a"}).compare(Category({"a"})));
    ASSERT_EQ(0, Category({"a","a"}).compare(Category({"a","a"})));
    ASSERT_EQ(0, Category({"a","a","a"}).compare(Category({"a","a","a"})));

    ASSERT_EQ(1, Category({"b"}).compare(Category({"a"})));
    ASSERT_EQ(1, Category({"b","a"}).compare(Category({"a","a"})));
    ASSERT_EQ(1, Category({"b","a","c"}).compare(Category({"b","a","b"})));

    ASSERT_EQ(-1, Category({"a"}).compare(Category({"b"})));
    ASSERT_EQ(-1, Category({"a","a"}).compare(Category({"a","b"})));
    ASSERT_EQ(-1, Category({"a","b"}).compare(Category({"a","b","c"})));
}