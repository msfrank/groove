#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>

#include <groove_model/column_traits.h>
#include <groove_model/indexed_column_template.h>
#include <groove_model/indexed_column_writer_template.h>
#include <groove_model/page_traits.h>
#include <groove_model/rocksdb_store.h>
#include <tempo_utils/tempdir_maker.h>

class CategoryInt64IndexedColumnTest : public ::testing::Test {
protected:
    tempo_utils::Url datasetUrl;
    std::shared_ptr<const std::string> columnId;
    std::shared_ptr<groove_data::CategoryInt64Vector> vector;

    void SetUp() override {
        datasetUrl = tempo_utils::Url::fromString("test://dataset");
        columnId = std::make_shared<const std::string>("cat");

        auto keyField = arrow::field(*columnId, groove_data::CategoryBuilder::makeDatatype());
        auto i64Field = arrow::field("", arrow::int64());
        auto emptyField = arrow::field("", arrow::boolean());
        auto schema = arrow::schema({keyField, i64Field, emptyField});
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

        arrow::BooleanBuilder emptyBuilder;
        TU_ASSERT (emptyBuilder.Append(false).ok());
        TU_ASSERT (emptyBuilder.Append(false).ok());
        TU_ASSERT (emptyBuilder.Append(false).ok());
        auto buildEmptyResult = emptyBuilder.Finish();
        TU_ASSERT (buildEmptyResult.ok());

        auto table = arrow::Table::Make(schema, {*buildKeyResult, *buildI64Result, *buildEmptyResult}, 3);
        vector = groove_data::CategoryInt64Vector::create(table, 0, 1, 2);
    }
};

TEST_F(CategoryInt64IndexedColumnTest, TestGetValuesFromEmptyPageStore)
{
    using namespace groove_model;

    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    auto pageStore = RocksDbStore::create(tempdirMaker.getTempdir());
    ASSERT_TRUE (pageStore->open().ok());

    auto modelId = std::make_shared<const std::string>("test");
    auto result = pageStore->getIndexedPage<CategoryInt64>(datasetUrl, modelId, columnId, Option<groove_data::Category>(), false);
    ASSERT_TRUE (result.isStatus());
    ASSERT_TRUE (result.getStatus().matchesCondition(groove_model::ModelCondition::kPageNotFound));
}

TEST_F(CategoryInt64IndexedColumnTest, TestSetAndGetValues)
{
    using namespace groove_model;

    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    auto pageStore = RocksDbStore::create(tempdirMaker.getTempdir());
    ASSERT_TRUE (pageStore->open().ok());

    auto modelId = std::make_shared<const std::string>("test");

    auto column = IndexedColumn<CategoryInt64>::create(datasetUrl, modelId, columnId, pageStore);
    auto writer = IndexedColumnWriter<CategoryInt64>::create(datasetUrl, modelId, columnId, pageStore);
    auto status = writer->setValues(vector);
    ASSERT_TRUE(status.isOk());

    groove_data::CategoryRange range;
    range.start = Option<groove_data::Category>(groove_data::Category({"a"}));
    range.start_exclusive = false;
    range.end = Option<groove_data::Category>(groove_data::Category({"b","c"}));
    range.end_exclusive = false;
    auto getValuesResult = column->getValues(range);
    ASSERT_TRUE (getValuesResult.isResult());

    auto values = getValuesResult.getResult();
    groove_data::CategoryInt64Datum datum;
    ASSERT_TRUE (values.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"a"}));
    ASSERT_EQ (datum.value, 4);
    ASSERT_TRUE (values.getNext(datum));
    ASSERT_EQ (datum.key, groove_data::Category({"b","c"}));
    ASSERT_EQ (datum.value, 5);
    ASSERT_FALSE (values.getNext(datum));

    std::filesystem::remove_all(tempdirMaker.getTempdir());
}