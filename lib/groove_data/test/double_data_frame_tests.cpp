#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>
#include <arrow/array/builder_binary.h>

#include <groove_data/double_frame.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>

class DoubleDataFrameTest : public ::testing::Test {
protected:
    std::shared_ptr<arrow::Schema> schema;
    std::shared_ptr<arrow::Table> table;

    void SetUp() override {

        auto keyField = arrow::field("key", arrow::float64());
        auto i64Field = arrow::field("i64", arrow::int64());
        auto dblField = arrow::field("dbl", arrow::float64());
        auto strField = arrow::field("str", arrow::utf8());
        schema = arrow::schema({keyField, i64Field, dblField, strField});
        TU_ASSERT (schema != nullptr);

        arrow::DoubleBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0.0).ok());
        TU_ASSERT (keyBuilder.Append(1.0).ok());
        TU_ASSERT (keyBuilder.Append(2.0).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::Int64Builder i64Builder;
        TU_ASSERT (i64Builder.Append(4).ok());
        TU_ASSERT (i64Builder.Append(5).ok());
        TU_ASSERT (i64Builder.Append(6).ok());
        auto buildI64Result = i64Builder.Finish();
        TU_ASSERT (buildI64Result.ok());

        arrow::DoubleBuilder dblBuilder;
        TU_ASSERT (dblBuilder.Append(7.0).ok());
        TU_ASSERT (dblBuilder.Append(8.0).ok());
        TU_ASSERT (dblBuilder.Append(9.0).ok());
        auto buildDblResult = dblBuilder.Finish();
        TU_ASSERT (buildDblResult.ok());

        arrow::StringBuilder strBuilder;
        TU_ASSERT (strBuilder.Append("ten").ok());
        TU_ASSERT (strBuilder.Append("eleven").ok());
        TU_ASSERT (strBuilder.Append("twelve").ok());
        auto buildStrResult = strBuilder.Finish();
        TU_ASSERT (buildStrResult.ok());

        table = arrow::Table::Make(schema, {*buildKeyResult, *buildI64Result, *buildDblResult, *buildStrResult}, 3);
    }
};

TEST_F(DoubleDataFrameTest, TestCreateFrame)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);
    ASSERT_EQ (frame->numVectors(), 3);

    auto smallestKey = frame->smallestKey();
    ASSERT_TRUE (!smallestKey.isEmpty());
    ASSERT_DOUBLE_EQ (smallestKey.getValue(), 0.0);

    auto largestKey = frame->largestKey();
    ASSERT_TRUE (!largestKey.isEmpty());
    ASSERT_DOUBLE_EQ (largestKey.getValue(), 2.0);
}

TEST_F(DoubleDataFrameTest, TestDoubleInt64GetDatum)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto dbli64 = std::static_pointer_cast<groove_data::DoubleInt64Vector>(vector);
    auto datum = dbli64->getDatum(1);
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_EQ (datum.value, 5);
}

TEST_F(DoubleDataFrameTest, TestDoubleInt64IterateValues)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto dbli64 = std::static_pointer_cast<groove_data::DoubleInt64Vector>(vector);
    auto iterator = dbli64->iterator();
    groove_data::DoubleInt64Datum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 0.0);
    ASSERT_EQ (datum.value, 4);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_EQ (datum.value, 5);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 2.0);
    ASSERT_EQ (datum.value, 6);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(DoubleDataFrameTest, TestDoubleDoubleGetDatum)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto dbldbl = std::static_pointer_cast<groove_data::DoubleDoubleVector>(vector);
    auto datum = dbldbl->getDatum(1);
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_DOUBLE_EQ(datum.value, 8.0);
}

TEST_F(DoubleDataFrameTest, TestDoubleDoubleIterateValues)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto dbldbl = std::static_pointer_cast<groove_data::DoubleDoubleVector>(vector);
    auto iterator = dbldbl->iterator();
    groove_data::DoubleDoubleDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 0.0);
    ASSERT_DOUBLE_EQ (datum.value, 7.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_DOUBLE_EQ (datum.value, 8.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 2.0);
    ASSERT_DOUBLE_EQ (datum.value, 9.0);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(DoubleDataFrameTest, TestDoubleStringGetDatum)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto dblstr = std::static_pointer_cast<groove_data::DoubleStringVector>(vector);
    auto datum = dblstr->getDatum(1);
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_EQ(datum.value, "eleven");
}

TEST_F(DoubleDataFrameTest, TestDoubleStringIterateValues)
{
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto dblstr = std::static_pointer_cast<groove_data::DoubleStringVector>(vector);
    auto iterator = dblstr->iterator();
    groove_data::DoubleStringDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 0.0);
    ASSERT_EQ (datum.value, "ten");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 1.0);
    ASSERT_EQ (datum.value, "eleven");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_DOUBLE_EQ (datum.key, 2.0);
    ASSERT_EQ (datum.value, "twelve");
    ASSERT_FALSE (iterator.getNext(datum));
}