#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>

#include <groove_data/double_frame.h>
#include <groove_data/int64_frame.h>
#include <groove_model/groove_database.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>
#include <tempo_utils/tempdir_maker.h>

class GrooveModelTest : public ::testing::Test {
protected:
    groove_model::GrooveSchema schema;

    void SetUp() override {
        groove_model::SchemaState state;

        groove_model::SchemaModel *model;
        TU_ASSIGN_OR_RAISE (model, state.putModel("model",
            groove_model::ModelKeyType::Double, groove_model::ModelKeyCollation::Indexed));
        groove_model::SchemaColumn *column;
        TU_ASSIGN_OR_RAISE (column, state.appendColumn("column",
            groove_model::ColumnValueType::Double, groove_model::ColumnValueFidelity::OnlyValidValue));

        model->appendColumn(column);

        auto toSchemaResult = state.toSchema();
        TU_ASSERT (toSchemaResult.isResult());
        schema = toSchemaResult.getResult();
    }

    std::shared_ptr<groove_data::BaseFrame> createValidFrame()
    {
        auto keyField = arrow::field("", arrow::float64());
        auto valField = arrow::field("column", arrow::float64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema_ = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema_ != nullptr);

        arrow::DoubleBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::DoubleBuilder valBuilder;
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

        auto table = arrow::Table::Make(schema_, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
        auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,2}});
        TU_ASSERT (createFrameResult.isResult());
        return createFrameResult.getResult();
    }

    std::shared_ptr<groove_data::BaseFrame> createFrameWithUnknownFieldId()
    {
        auto keyField = arrow::field("", arrow::float64());
        auto valField = arrow::field("unknowncolumn", arrow::float64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema_ = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema_ != nullptr);

        arrow::DoubleBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::DoubleBuilder valBuilder;
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

        auto table = arrow::Table::Make(schema_, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
        auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,2}});
        TU_ASSERT (createFrameResult.isResult());
        return createFrameResult.getResult();
    }

    std::shared_ptr<groove_data::BaseFrame> createFrameWithWrongValueType()
    {
        auto keyField = arrow::field("", arrow::float64());
        auto valField = arrow::field("column", arrow::int64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema_ = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema_ != nullptr);

        arrow::DoubleBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::DoubleBuilder valBuilder;
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

        auto table = arrow::Table::Make(schema_, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
        auto createFrameResult = groove_data::DoubleFrame::create(table, 0, {{1,2}});
        TU_ASSERT (createFrameResult.isResult());
        return createFrameResult.getResult();
    }

    std::shared_ptr<groove_data::BaseFrame> createFrameWithWrongKeyType()
    {
        auto keyField = arrow::field("", arrow::int64());
        auto valField = arrow::field("column", arrow::float64());
        auto fidField = arrow::field("", arrow::boolean());
        auto schema_ = arrow::schema({keyField, valField, fidField});
        TU_ASSERT (schema_ != nullptr);

        arrow::DoubleBuilder keyBuilder;
        TU_ASSERT (keyBuilder.Append(0).ok());
        TU_ASSERT (keyBuilder.Append(1).ok());
        TU_ASSERT (keyBuilder.Append(2).ok());
        auto buildKeyResult = keyBuilder.Finish();
        TU_ASSERT (buildKeyResult.ok());

        arrow::DoubleBuilder valBuilder;
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

        auto table = arrow::Table::Make(schema_, {*buildKeyResult, *buildValResult, *buildFidResult}, 3);
        auto createFrameResult = groove_data::Int64Frame::create(table, 0, {{1,2}});
        TU_ASSERT (createFrameResult.isResult());
        return createFrameResult.getResult();
    }
};

TEST_F(GrooveModelTest, UpdateModelSucceeds)
{
    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    using namespace groove_model;
    DatabaseOptions options;
    options.modelsDirectory = tempdirMaker.getTempdir();
    auto db = std::make_shared<groove_model::GrooveDatabase>(options);
    ASSERT_TRUE (db->configure().isOk());

    auto datasetUrl = tempo_utils::Url::fromString("test:/");
    ASSERT_TRUE (db->declareDataset(datasetUrl, schema).isOk());

    std::vector<std::string> failedVectors;
    auto status = db->updateModel(datasetUrl, "model", createValidFrame(), &failedVectors);
    ASSERT_TRUE (status.isOk());
    ASSERT_EQ (std::vector<std::string>{}, failedVectors);

    std::filesystem::remove_all(tempdirMaker.getTempdir());
}

TEST_F(GrooveModelTest, UpdateModelFailsUnknownFieldId)
{
    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    using namespace groove_model;
    DatabaseOptions options;
    options.modelsDirectory = tempdirMaker.getTempdir();
    auto db = std::make_shared<groove_model::GrooveDatabase>(options);
    ASSERT_TRUE (db->configure().isOk());

    auto datasetUrl = tempo_utils::Url::fromString("test:/");
    ASSERT_TRUE (db->declareDataset(datasetUrl, schema).isOk());

    std::vector<std::string> failedVectors;
    auto status = db->updateModel(datasetUrl, "model", createFrameWithUnknownFieldId(), &failedVectors);
    ASSERT_TRUE (status.isOk());
    ASSERT_EQ (std::vector<std::string>{std::string{"unknowncolumn"}}, failedVectors);

    std::filesystem::remove_all(tempdirMaker.getTempdir());
}

TEST_F(GrooveModelTest, UpdateModelFailsWrongKeyType)
{
    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    using namespace groove_model;
    DatabaseOptions options;
    options.modelsDirectory = tempdirMaker.getTempdir();
    auto db = std::make_shared<groove_model::GrooveDatabase>(options);
    ASSERT_TRUE (db->configure().isOk());

    auto datasetUrl = tempo_utils::Url::fromString("test:/");
    ASSERT_TRUE (db->declareDataset(datasetUrl, schema).isOk());

    std::vector<std::string> failedVectors;
    auto status = db->updateModel(datasetUrl, "model", createFrameWithWrongKeyType(), &failedVectors);
    ASSERT_FALSE (status.isOk());

    std::filesystem::remove_all(tempdirMaker.getTempdir());
}

TEST_F(GrooveModelTest, UpdateModelFailsWrongValueType)
{
    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    using namespace groove_model;
    DatabaseOptions options;
    options.modelsDirectory = tempdirMaker.getTempdir();
    auto db = std::make_shared<groove_model::GrooveDatabase>(options);
    ASSERT_TRUE (db->configure().isOk());

    auto datasetUrl = tempo_utils::Url::fromString("test:/");
    ASSERT_TRUE (db->declareDataset(datasetUrl, schema).isOk());

    std::vector<std::string> failedVectors;
    auto status = db->updateModel(datasetUrl, "model", createFrameWithWrongValueType(), &failedVectors);
    ASSERT_TRUE (status.isOk());
    ASSERT_EQ (std::vector<std::string>{std::string{"column"}}, failedVectors);

    std::filesystem::remove_all(tempdirMaker.getTempdir());
}