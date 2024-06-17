#include <gtest/gtest.h>

#include <groove_units/unit_reference_system.h>

class UnitDimension : public ::testing::Test {
protected:
    groove_units::UnitReferenceSystem rs;

    void
    SetUp() override
    {
        rs = groove_units::UnitReferenceSystem("test",
            { // base units
                "g", "m", "s", "b", "B"
            },
            { // scale products
                {"mega",    10, 6},
                {"kilo",    10, 3},
                {"deci",    10, -1},
                {"centi",   10, -2},
                {"milli",   10, -3}
            }
        );
    }
};

TEST_F(UnitDimension, TestScaleUnitDimensionWhenFixedHasScaleProduct)
{
    auto fixed = rs.parse("milli s").getResult();
    auto target = rs.parse("s").getResult();

    int base = 1;
    int exponent = 1;
    auto status = fixed.scale(target, &base, &exponent);
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(base, 10);
    ASSERT_EQ(exponent, 3);
}

TEST_F(UnitDimension, TestScaleUnitDimensionWhenTargetHasScaleProduct)
{
    auto fixed = rs.parse("s").getResult();
    auto target = rs.parse("milli s").getResult();

    int base = 1;
    int exponent = 1;
    auto status = fixed.scale(target, &base, &exponent);
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(base, 10);
    ASSERT_EQ(exponent, -3);
}

TEST_F(UnitDimension, TestScaleUnitDimensionWhenBothHaveScaleProduct)
{
    auto fixed = rs.parse("mega s").getResult();
    auto target = rs.parse("milli s").getResult();

    int base = 1;
    int exponent = 1;
    auto status = fixed.scale(target, &base, &exponent);
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(base, 10);
    ASSERT_EQ(exponent, -9);
    // fixed: 10^6, target: 10^-3 =  fixed - target = 6 - (-3) = 9
    // target < 0 = -(9)
}