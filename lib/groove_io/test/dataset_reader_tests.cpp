#include <gtest/gtest.h>

#include <groove_data/double_frame.h>
#include <groove_io/dataset_reader.h>
#include <groove_io/dataset_writer.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>
#include <tempo_utils/tempdir_maker.h>

class DatasetReaderTest : public ::testing::Test {
protected:
    tempo_utils::Url datasetUrl;
    std::string modelId;
    std::string columnId;
    std::filesystem::path datasetDir;
    std::filesystem::path datasetFile;

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
        auto schema = toSchemaResult.getResult();

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
        auto frame = createFrameResult.getResult();

        tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "test_DatasetWriter.XXXXXXXX");
        TU_ASSERT (tempdirMaker.isValid());

        groove_io::DatasetWriter writer(datasetUrl, schema);
        writer.putFrame(modelId, frame);
        datasetDir = tempdirMaker.getTempdir();
        datasetFile = datasetDir / "dataset.gds";

        auto status = writer.writeDataset(datasetFile);
        TU_ASSERT (status.isOk());
        TU_ASSERT (exists(datasetFile));
    }
};

TEST_F(DatasetReaderTest, TestReadDataset)
{
    auto createReaderResult = groove_io::DatasetReader::create(datasetFile);
    ASSERT_TRUE (createReaderResult.isResult());

    auto reader = createReaderResult.getResult();
    ASSERT_TRUE (reader->isValid());
    ASSERT_FALSE (reader->isEmpty());

    auto getIndexedPageResult = reader->getIndexedPage<groove_model::DoubleDouble>(
        datasetUrl,
        std::make_shared<const std::string>(modelId),
        std::make_shared<const std::string>(columnId),
        Option<double>(0.0),
        false);
    ASSERT_TRUE (getIndexedPageResult.isResult());

    auto page = getIndexedPageResult.getResult();
    ASSERT_EQ (3, page->numRows());

    groove_data::DoubleDoubleDatum datum;
    ASSERT_TRUE (page->getDatum(0.0, datum));
    ASSERT_DOUBLE_EQ (4.0, datum.value);
    ASSERT_TRUE (page->getDatum(1.0, datum));
    ASSERT_DOUBLE_EQ (5.0, datum.value);
    ASSERT_TRUE (page->getDatum(2.0, datum));
    ASSERT_DOUBLE_EQ (6.0, datum.value);

    ASSERT_TRUE (std::filesystem::remove_all(datasetDir));
}