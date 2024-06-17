#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>
#include <arrow/array/builder_binary.h>
#include <arrow/array/builder_nested.h>

#include <groove_data/category_frame.h>
#include <groove_data/category_double_vector.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/category_builder.h>

class CategoryDataFrameTest : public ::testing::Test {
protected:
    std::shared_ptr<arrow::Schema> schema;
    std::shared_ptr<arrow::Table> table;

    void SetUp() override {

        auto keyField = arrow::field("cat", groove_data::CategoryBuilder::makeDatatype());
        auto i64Field = arrow::field("i64", arrow::int64());
        auto dblField = arrow::field("dbl", arrow::float64());
        auto strField = arrow::field("str", arrow::utf8());
        schema = arrow::schema({keyField, i64Field, dblField, strField});
        TU_ASSERT (schema != nullptr);

        groove_data::CategoryBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"a"})).ok());
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"b","c"})).ok());
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"b","d","e"})).ok());
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

TEST_F(CategoryDataFrameTest, TestCreateFrame)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);
    ASSERT_EQ (frame->numVectors(), 3);

    auto smallestKey = frame->smallestKey();
    ASSERT_TRUE (!smallestKey.isEmpty());
    ASSERT_EQ (smallestKey.getValue(), groove_data::Category({"a"}));

    auto largestKey = frame->largestKey();
    ASSERT_TRUE (!largestKey.isEmpty());
    ASSERT_EQ (largestKey.getValue(), groove_data::Category({"b", "d", "e"}));
}

TEST_F(CategoryDataFrameTest, TestDoubleInt64GetDatum)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto cati64 = std::static_pointer_cast<groove_data::CategoryInt64Vector>(vector);
    auto datum = cati64->getDatum(1);
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_EQ (datum.value, 5);
}

TEST_F(CategoryDataFrameTest, TestDoubleInt64IterateValues)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("i64");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_INT64);
    ASSERT_EQ (vector->getSize(), 3);

    auto cati64 = std::static_pointer_cast<groove_data::CategoryInt64Vector>(vector);
    auto iterator = cati64->iterator();
    groove_data::CategoryInt64Datum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"a"}));
    ASSERT_EQ (datum.value, 4);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_EQ (datum.value, 5);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "d", "e"}));
    ASSERT_EQ (datum.value, 6);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(CategoryDataFrameTest, TestDoubleDoubleGetDatum)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto catdbl = std::static_pointer_cast<groove_data::CategoryDoubleVector>(vector);
    auto datum = catdbl->getDatum(1);
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_DOUBLE_EQ(datum.value, 8.0);
}

TEST_F(CategoryDataFrameTest, TestDoubleDoubleIterateValues)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("dbl");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE);
    ASSERT_EQ (vector->getSize(), 3);

    auto catdbl = std::static_pointer_cast<groove_data::CategoryDoubleVector>(vector);
    auto iterator = catdbl->iterator();
    groove_data::CategoryDoubleDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"a"}));
    ASSERT_DOUBLE_EQ (datum.value, 7.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_DOUBLE_EQ (datum.value, 8.0);
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "d", "e"}));
    ASSERT_DOUBLE_EQ (datum.value, 9.0);
    ASSERT_FALSE (iterator.getNext(datum));
}

TEST_F(CategoryDataFrameTest, TestDoubleStringGetDatum)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto catstr = std::static_pointer_cast<groove_data::CategoryStringVector>(vector);
    auto datum = catstr->getDatum(1);
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_EQ(datum.value, "eleven");
}

TEST_F(CategoryDataFrameTest, TestDoubleStringIterateValues)
{
    auto createFrameResult = groove_data::CategoryFrame::create(table, 0, {{1,-1},{2,-1},{3,-1}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    ASSERT_TRUE (frame != nullptr);

    auto vector = frame->getVector("str");
    ASSERT_TRUE (vector != nullptr);
    ASSERT_EQ (vector->getVectorType(), groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_STRING);
    ASSERT_EQ (vector->getSize(), 3);

    auto catstr = std::static_pointer_cast<groove_data::CategoryStringVector>(vector);
    auto iterator = catstr->iterator();
    groove_data::CategoryStringDatum datum;

    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"a"}));
    ASSERT_EQ (datum.value, "ten");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "c"}));
    ASSERT_EQ (datum.value, "eleven");
    ASSERT_TRUE (iterator.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b", "d", "e"}));
    ASSERT_EQ (datum.value, "twelve");
    ASSERT_FALSE (iterator.getNext(datum));
}