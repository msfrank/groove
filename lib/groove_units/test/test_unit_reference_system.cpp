#include <gtest/gtest.h>

#include <groove_units/unit_reference_system.h>


class UnitReferenceSystemFixture : public ::testing::Test {
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

TEST_F(UnitReferenceSystemFixture, TestParseSingleUnitNoScaleProduct)
{
    auto result = rs.parse("s");
    ASSERT_TRUE(result.isResult());
    //TU_LOG_INFO << "result:" << result.getResult();
}

TEST_F(UnitReferenceSystemFixture, TestParseSingleUnitWithScaleProduct)
{
    auto result = rs.parse("milli s");
    ASSERT_TRUE(result.isResult());
    //TU_LOG_INFO << "result:" << result.getResult();
}

TEST_F(UnitReferenceSystemFixture, TestParseComplexUnitWithScaleProduct)
{
    auto result = rs.parse("kilo g * m / s^2");
    ASSERT_TRUE(result.isResult());
    //TU_LOG_INFO << "result:" << result.getResult();
}