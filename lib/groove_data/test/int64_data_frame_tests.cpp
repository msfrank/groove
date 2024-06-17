#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>
#include <arrow/array/builder_binary.h>

#include <groove_data/int64_frame.h>
#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>

class Int64DataFrameTest : public ::testing::Test {
protected:
    std::shared_ptr<arrow::Schema> schema;
    std::shared_ptr<arrow::Table> table;

    void SetUp() override {

        auto keyField = arrow::field("", arrow::int64());
        auto i64Field = arrow::field("i64", arrow::int64());
        auto dblField = arrow::field("dbl", arrow::float64());
        auto strField = arrow::field("str", arrow::utf8());
        auto emptyField = arrow::field("", arrow::boolean());
        schema = arrow::schema({keyField, i64Field, emptyField, dblField, emptyField, strField, emptyField});
        TU_ASSERT (schema != nullptr);

        arrow::Int64Builder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
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

        arrow::BooleanBuilder emptyBuilder;
        TU_ASSERT (emptyBuilder.Append(false).ok());
        TU_ASSERT (emptyBuilder.Append(false).ok());
        TU_ASSERT (emptyBuilder.Append(false).ok());
        auto buildEmptyResult = emptyBuilder.Finish();
        TU_ASSERT (buildEmptyResult.ok());

        table = arrow::Table::Make(schema, {
            *buildKeyResult,
            *buildI64Result, *buildEmptyResult,
            *buildDblResult, *buildEmptyResult,
            *buildStrResult, *buildEmptyResult
            }, 3);
    }
};

TEST_F(Int64DataFrameTest, TestCreateFrame)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);
    ASSERT_EQ (frame->numVectors(), 3);

    auto smallestKey = frame->smallestKey();
    ASSERT_TRUE (!smallestKey.isEmpty());
    ASSERT_EQ (smallestKey.getValue(), 0);

    auto largestKey = frame->largestKey();
    ASSERT_TRUE (!largestKey.isEmpty());
    ASSERT_EQ (largestKey.getValue(), 2);
}

TEST_F(Int64DataFrameTest, TestInt64Int64GetDatum)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64i64 = std::static_pointer_cast<groove_data::Int64Int64Vector>(vector);
    auto datum = i64i64->getDatum(1);
    ASSERT_EQ (datum.key, 1);
    ASSERT_EQ (datum.value, 5);
}

TEST_F(Int64DataFrameTest, TestInt64Int64IterateValues)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64i64 = std::static_pointer_cast<groove_data::Int64Int64Vector>(vector);
    auto iterator = i64i64->iterator();
    groove_data::Int64Int64Datum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 0);
    ASSERT_EQ (datum.value, 4);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 1);
    ASSERT_EQ (datum.value, 5);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 2);
    ASSERT_EQ (datum.value, 6);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(Int64DataFrameTest, TestInt64Int64GetSlice)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64i64 = std::static_pointer_cast<groove_data::Int64Int64Vector>(vector);
    groove_data::Int64Range range;
    range.start = Option<tu_int64>(1);
    range.start_exclusive = false;
    range.end = Option<tu_int64>();
    range.end_exclusive = false;
    auto slice = i64i64->slice(range);
    ASSERT_EQ (slice->getSize(), 2);
}

TEST_F(Int64DataFrameTest, TestInt64DoubleGetDatum)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64dbl = std::static_pointer_cast<groove_data::Int64DoubleVector>(vector);
    auto datum = i64dbl->getDatum(1);
    ASSERT_EQ (datum.key, 1);
    ASSERT_DOUBLE_EQ(datum.value, 8.0);
}

TEST_F(Int64DataFrameTest, TestInt64DoubleIterateValues)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64dbl = std::static_pointer_cast<groove_data::Int64DoubleVector>(vector);
    auto iterator = i64dbl->iterator();
    groove_data::Int64DoubleDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 0);
    ASSERT_DOUBLE_EQ (datum.value, 7.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 1);
    ASSERT_DOUBLE_EQ (datum.value, 8.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 2);
    ASSERT_DOUBLE_EQ (datum.value, 9.0);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(Int64DataFrameTest, TestInt64DoubleGetSlice)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64dbl = std::static_pointer_cast<groove_data::Int64DoubleVector>(vector);
    groove_data::Int64Range range;
    range.start = Option<tu_int64>(1);
    range.start_exclusive = false;
    range.end = Option<tu_int64>();
    range.end_exclusive = false;
    auto slice = i64dbl->slice(range);
    ASSERT_EQ (slice->getSize(), 2);
}

TEST_F(Int64DataFrameTest, TestInt64StringGetDatum)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64str = std::static_pointer_cast<groove_data::Int64StringVector>(vector);
    auto datum = i64str->getDatum(1);
    ASSERT_EQ (datum.key, 1);
    ASSERT_EQ(datum.value, "eleven");
}

TEST_F(Int64DataFrameTest, TestInt64StringIterateValues)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64str = std::static_pointer_cast<groove_data::Int64StringVector>(vector);
    auto iterator = i64str->iterator();
    groove_data::Int64StringDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 0);
    ASSERT_EQ (datum.value, "ten");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 1);
    ASSERT_EQ (datum.value, "eleven");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, 2);
    ASSERT_EQ (datum.value, "twelve");
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(Int64DataFrameTest, TestInt64StringGetSlice)
{
    auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2},{3,4},{5,6}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_INT64_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto i64str = std::static_pointer_cast<groove_data::Int64StringVector>(vector);
    groove_data::Int64Range range;
    range.start = Option<tu_int64>(1);
    range.start_exclusive = false;
    range.end = Option<tu_int64>();
    range.end_exclusive = false;
    auto slice = i64str->slice(range);
    ASSERT_EQ (slice->getSize(), 2);
}