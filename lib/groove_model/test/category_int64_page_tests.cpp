#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>

#include <groove_data/category_int64_vector.h>
#include <groove_model/indexed_page_template.h>
#include <groove_model/page_traits.h>

class CategoryInt64PageTest : public ::testing::Test {
protected:
    tempo_utils::Url datasetUrl;
    std::shared_ptr<groove_data::CategoryInt64Vector> vector;

    void SetUp() override {
        datasetUrl = tempo_utils::Url::fromString("test://dataset");

        auto keyField = arrow::field("cat", arrow::list(arrow::utf8()));
        auto valField = arrow::field("", arrow::int64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema != nullptr);

        groove_data::CategoryBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"a"})).ok());
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"b","c"})).ok());
        TU_ASSERT (keyBuilder.Append(groove_data::Category({"b","d","e"})).ok());
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
        vector = groove_data::CategoryInt64Vector::create(table, 0, 1, 2);
    }
};

TEST_F(CategoryInt64PageTest, TestCreatePageFromVector)
{
    using namespace groove_model;
    auto pageId = PageId::create<CategoryInt64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto page = IndexedPage<CategoryInt64>::fromVector(pageId, vector);
    ASSERT_TRUE (page != nullptr);
    ASSERT_EQ (page->numRows(), 3);
}

TEST_F(CategoryInt64PageTest, TestWritePageToBuffer)
{
    using namespace groove_model;
    auto pageId = PageId::create<CategoryInt64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto page = IndexedPage<CategoryInt64>::fromVector(pageId, vector);
    ASSERT_TRUE (page != nullptr);
    auto buffer = page->toBuffer();
    ASSERT_TRUE (buffer != nullptr);
}

TEST_F(CategoryInt64PageTest, TestReadPageFromBytes)
{
    using namespace groove_model;
    auto pageId = PageId::create<CategoryInt64,groove_data::CollationMode::COLLATION_INDEXED>(
        datasetUrl,
        std::make_shared<const std::string>("foo"),
        std::make_shared<const std::string>("foo"),
        Option<tu_int64>());
    auto src = IndexedPage<CategoryInt64>::fromVector(pageId, vector);
    ASSERT_TRUE (src != nullptr);
    auto buffer = src->toBuffer();
    auto bytes = std::make_shared<const std::string>(buffer->ToString());
    auto dst = IndexedPage<CategoryInt64>::fromBytes(pageId, bytes);
    ASSERT_TRUE (dst != nullptr);
    ASSERT_EQ (dst->numRows(), 3);
    auto datum = dst->getVector()->getDatum(0);
    ASSERT_EQ (groove_data::Category({"a"}), datum.key);
    ASSERT_EQ (4, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
    datum = dst->getVector()->getDatum(1);
    ASSERT_EQ (groove_data::Category({"b","c"}), datum.key);
    ASSERT_EQ (5, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
    datum = dst->getVector()->getDatum(2);
    ASSERT_EQ (groove_data::Category({"b","d","e"}), datum.key);
    ASSERT_EQ (6, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
    datum = dst->getVector()->getDatum(3);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_INVALID, datum.fidelity);
}