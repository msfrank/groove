#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/algorithm/string/predicate.hpp>

#include <groove_agent/storage_supervisor.h>
#include <groove_data/double_frame.h>
#include <groove_model/groove_database.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>
#include <tempo_utils/tempdir_maker.h>

class StorageSupervisorTest : public ::testing::Test {
protected:
    std::filesystem::path dbPath;
    std::shared_ptr<groove_model::GrooveDatabase> db;
    groove_model::GrooveSchema dataset;
    tempo_utils::Url datasetUrl;

    void SetUp() override
    {
        tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
        ASSERT_TRUE (tempdirMaker.isValid());
        dbPath = tempdirMaker.getTempdir();

        using namespace groove_model;
        DatabaseOptions options;
        options.modelsDirectory = dbPath;
        db = std::make_shared<groove_model::GrooveDatabase>(options);
        ASSERT_TRUE (db->configure().isOk());
    }

    void TearDown() override
    {
        std::filesystem::remove_all(dbPath);
    }
};

TEST_F(StorageSupervisorTest, CreateCollection)
{
    StorageSupervisor supervisor(db.get());

    auto createCollectionResult = supervisor.createCollection("test");
    ASSERT_TRUE (createCollectionResult.isResult());
    auto collection = createCollectionResult.getResult();
    auto collectionUrl = collection->getCollectionUrl();
    ASSERT_EQ (tempo_utils::Url::fromString("/test"), collectionUrl);
}

TEST_F(StorageSupervisorTest, CreateEphemeralCollection)
{
    StorageSupervisor supervisor(db.get());

    auto createCollectionResult = supervisor.createEphemeralCollection("test");
    ASSERT_TRUE (createCollectionResult.isResult());
    auto collection = createCollectionResult.getResult();
    auto collectionUrl = collection->getCollectionUrl();
    ASSERT_TRUE (boost::algorithm::ends_with(collectionUrl.getPath(), "-test"));
}

TEST_F(StorageSupervisorTest, CreateDataset)
{
    StorageSupervisor supervisor(db.get());

    auto createCollectionResult = supervisor.createEphemeralCollection("test");
    ASSERT_TRUE (createCollectionResult.isResult());
    auto collection = createCollectionResult.getResult();
    auto collectionUrl = collection->getCollectionUrl();

    //
    groove_model::SchemaState state;

    groove_model::SchemaModel *model;
    TU_ASSIGN_OR_RAISE (model, state.putModel("foo",
        groove_model::ModelKeyType::Category, groove_model::ModelKeyCollation::Indexed));

    groove_model::SchemaColumn *column;
    TU_ASSIGN_OR_RAISE (column, state.appendColumn("foo1",
        groove_model::ColumnValueType::Double, groove_model::ColumnValueFidelity::OnlyValidValue));

    TU_RAISE_IF_NOT_OK (model->appendColumn(column));

    auto toSchemaResult = state.toSchema();
    ASSERT_TRUE (toSchemaResult.isResult());
    auto schema = toSchemaResult.getResult();

    auto datasetUrl = collectionUrl.traverse(tempo_utils::UrlPathPart("test"));

    auto createDatasetResult = supervisor.createDataset(datasetUrl, schema);
    ASSERT_TRUE (createDatasetResult.isResult());
}

TEST_F(StorageSupervisorTest, PutData)
{
    StorageSupervisor supervisor(db.get());

    auto createCollectionResult = supervisor.createEphemeralCollection("test");
    ASSERT_TRUE (createCollectionResult.isResult());
    auto collection = createCollectionResult.getResult();
    auto collectionUrl = collection->getCollectionUrl();

    //
    groove_model::SchemaState state;

    groove_model::SchemaModel *model;
    TU_ASSIGN_OR_RAISE (model, state.putModel("foo",
        groove_model::ModelKeyType::Double, groove_model::ModelKeyCollation::Indexed));

    groove_model::SchemaColumn *column;
    TU_ASSIGN_OR_RAISE (column, state.appendColumn("foo1",
        groove_model::ColumnValueType::Double, groove_model::ColumnValueFidelity::OnlyValidValue));

    TU_RAISE_IF_NOT_OK (model->appendColumn(column));

    auto toSchemaResult = state.toSchema();
    ASSERT_TRUE (toSchemaResult.isResult());
    auto schema = toSchemaResult.getResult();

    auto datasetUrl = collectionUrl.traverse(tempo_utils::UrlPathPart("test"));

    auto createDatasetResult = supervisor.createDataset(datasetUrl, schema);
    ASSERT_TRUE (createDatasetResult.isResult());

    auto keyField = arrow::field("", arrow::float64());
    auto valField = arrow::field("foo1", arrow::float64());
    auto fidField = arrow::field("", arrow::boolean());
    auto schema_ = arrow::schema({keyField, valField, fidField});
    ASSERT_TRUE (schema_ != nullptr);

    arrow::DoubleBuilder keyBuilder;
    ASSERT_TRUE (keyBuilder.Append(0).ok());
    ASSERT_TRUE (keyBuilder.Append(1).ok());
    ASSERT_TRUE (keyBuilder.Append(2).ok());
    auto buildKeyResult = keyBuilder.Finish();
    ASSERT_TRUE (buildKeyResult.ok());

    arrow::DoubleBuilder valBuilder;
    ASSERT_TRUE (valBuilder.Append(4).ok());
    ASSERT_TRUE (valBuilder.Append(5).ok());
    ASSERT_TRUE (valBuilder.Append(6).ok());
    auto buildValResult = valBuilder.Finish();
    ASSERT_TRUE (buildValResult.ok());

    arrow::BooleanBuilder fidBuilder;
    ASSERT_TRUE (fidBuilder.Append(false).ok());
    ASSERT_TRUE (fidBuilder.Append(false).ok());
    ASSERT_TRUE (fidBuilder.Append(false).ok());
    auto buildFidResult = fidBuilder.Finish();
    ASSERT_TRUE (buildFidResult.ok());

    auto table = arrow::Table::Make(schema_, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
    auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,2}});
    ASSERT_TRUE (createFrameResult.isResult());
    auto frame = createFrameResult.getResult();
    auto putDataResult = supervisor.putData(datasetUrl, "foo", frame);
    ASSERT_TRUE (putDataResult.isResult());
    auto failedVectors = putDataResult.getResult();
    ASSERT_TRUE (failedVectors.empty());
}
