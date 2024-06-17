#include <gtest/gtest.h>

#include <groove_data/double_frame.h>
#include <groove_io/dataset_writer.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>
#include <tempo_utils/tempdir_maker.h>

class DatasetWriterTest : public ::testing::Test {
protected:
    tempo_utils::Url datasetUrl;
    std::string modelId;
    std::string columnId;
    groove_model::GrooveSchema schema;

    void SetUp() override {
        datasetUrl = tempo_utils::Url::fromString("test://dataset");
        modelId = "model";
        columnId = "column";

        groove_model::SchemaState state;
        groove_model::SchemaModel *model;
        TU_ASSIGN_OR_RAISE (model, state.putModel(modelId,
            groove_model::ModelKeyType::Double, groove_model::ModelKeyCollation::Indexed));
        groove_model::SchemaColumn *column;
        TU_ASSIGN_OR_RAISE (column, state.appendColumn(columnId,
            groove_model::ColumnValueType::Double, groove_model::ColumnValueFidelity::OnlyValidValue));
        model->appendColumn(column);

        auto toSchemaResult = state.toSchema();
        TU_ASSERT (toSchemaResult.isResult());
        schema = toSchemaResult.getResult();
    }

    std::shared_ptr<groove_data::BaseFrame> createValidFrame()
    {
        auto keyField = arrow::field("", arrow::float64());
        auto valField = arrow::field(columnId, arrow::float64());
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
};

TEST_F(DatasetWriterTest, TestWriteDataset)
{
    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "test_DatasetWriter.XXXXXXXX");
    ASSERT_TRUE (tempdirMaker.isValid());

    groove_io::DatasetWriter writer(datasetUrl, schema);
    auto frame = createValidFrame();
    writer.putFrame(modelId, frame);
    auto datasetPath = tempdirMaker.getTempdir() / "dataset.gds";

    auto status = writer.writeDataset(datasetPath);
    ASSERT_TRUE (status.isOk());
    ASSERT_TRUE (exists(datasetPath));
    ASSERT_TRUE (remove(datasetPath));
}