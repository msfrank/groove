#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>

#include <groove_data/int64_int64_vector.h>
#include <groove_model/indexed_page_template.h>
#include <groove_model/page_traits.h>

class Int64Int64PageTest : public ::testing::Test {
protected:
    tempo_utils::Url datasetUrl;
    std::shared_ptr<groove_data::Int64Int64Vector> vector;

    void SetUp() override {
        datasetUrl = tempo_utils::Url::fromString("test://dataset");

        auto keyField = arrow::field("i64", arrow::int64());
        auto valField = arrow::field("", arrow::int64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema != nullptr);

        arrow::Int64Builder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::Int64Builder valBuilder;
        TU_ASSERT (valBuilder.Append(4).ok());
        TU_ASSERT (valBuilder.Append(5).ok());
        TU_ASSERT (valBuilder.Append(6).ok());
        auto buildValResult = valBuilder.Finish();
        TU_ASSERT (buildValResult.ok());

        arrow::BooleanBuilder fidBuilder;
        TU_ASSERT (fidBuilder.Append(false).ok());
        TU_ASSERT (fidBuilder.Append(false).ok());
        TU_ASSERT (fidBuilder.Append(false).ok());
        auto buildFidResult = fidBuilder.Finish();
        TU_ASSERT (buildFidResult.ok());

        auto table = arrow::Table::Make(schema, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
        vector = groove_data::Int64Int64Vector::create(table, 0, 1, 2);
    }
};

TEST_F(Int64Int64PageTest, TestCreatePageFromVector)
{
    using namespace groove_model;
    auto pageId = PageId::create<Int64Int64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto page = IndexedPage<Int64Int64>::fromVector(pageId, vector);
    ASSERT_TRUE (page != nullptr);
    ASSERT_EQ (page->numRows(), 3);
}

TEST_F(Int64Int64PageTest, TestWritePageToBuffer)
{
    using namespace groove_model;
    auto pageId = PageId::create<Int64Int64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto page = IndexedPage<Int64Int64>::fromVector(pageId, vector);
    ASSERT_TRUE (page != nullptr);
    auto buffer = page->toBuffer();
    ASSERT_TRUE (buffer != nullptr);
}

TEST_F(Int64Int64PageTest, TestReadPageFromBytes)
{
    using namespace groove_model;
    auto pageId = PageId::create<Int64Int64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto src = IndexedPage<Int64Int64>::fromVector(pageId, vector);
    ASSERT_TRUE (src != nullptr);
    auto buffer = src->toBuffer();
    auto bytes = std::make_shared<const std::string>(buffer->ToString());
    auto dst = IndexedPage<Int64Int64>::fromBytes(pageId, bytes);
    ASSERT_TRUE (dst != nullptr);
    ASSERT_EQ (dst->numRows(), 3);
}