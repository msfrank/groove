#include <gtest/gtest.h>

#include <arrow/table_builder.h>
#include <arrow/array/builder_primitive.h>

#include <groove_data/double_frame.h>
#include <groove_model/column_traits.h>
#include <groove_model/groove_database.h>
#include <groove_model/indexed_column_template.h>
#include <groove_model/page_traits.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>
#include <groove_shapes/series_group_shape.h>
#include <tempo_utils/tempdir_maker.h>

class SeriesGroupShapeTest : public ::testing::Test {
protected:
    std::filesystem::path dbPath;
    std::shared_ptr<groove_model::GrooveDatabase> db;
    groove_model::GrooveSchema schema;
    tempo_utils::Url datasetUrl;

    void SetUp() override
    {
        auto keyField = arrow::field("", arrow::float64());
        auto valField = arrow::field("foo1", arrow::float64());
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
        ASSERT_TRUE (createFrameResult.isResult());
        auto frame = createFrameResult.getResult();

        groove_model::SchemaState state;

        groove_model::SchemaModel *model;
        TU_ASSIGN_OR_RAISE (model, state.putModel("foo",
            groove_model::ModelKeyType::Double, groove_model::ModelKeyCollation::Indexed));

        groove_model::SchemaColumn *column;
        TU_ASSIGN_OR_RAISE (column, state.appendColumn("foo1",
            groove_model::ColumnValueType::Double, groove_model::ColumnValueFidelity::OnlyValidValue));

        TU_RAISE_IF_NOT_OK (model->appendColumn(column));

        auto toSchemaResult = state.toSchema();
        TU_ASSERT (toSchemaResult.isResult());
        schema = toSchemaResult.getResult();

        tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "store.XXXXXXXX");
        ASSERT_TRUE (tempdirMaker.isValid());
        dbPath = tempdirMaker.getTempdir();

        using namespace groove_model;
        DatabaseOptions options;
        options.modelsDirectory = dbPath;
        db = std::make_shared<groove_model::GrooveDatabase>(options);
        ASSERT_TRUE (db->configure().isOk());

        datasetUrl = tempo_utils::Url::fromString("test:/");
        ASSERT_TRUE (db->declareDataset(datasetUrl, schema).isOk());
        auto dbDataset = db->getDataset(datasetUrl);
        ASSERT_TRUE (dbDataset != nullptr);
        auto dbModel = dbDataset->getModel("foo");
        ASSERT_TRUE (dbModel != nullptr);

        std::vector<std::string> failedVectors;
        ASSERT_TRUE (db->updateModel(datasetUrl, "foo", frame, &failedVectors).isOk());
        ASSERT_EQ (std::vector<std::string>{}, failedVectors);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(dbPath);
    }
};

TEST_F(SeriesGroupShapeTest, ConfigureSeriesGroupShape)
{
    groove_shapes::SeriesGroupShape seriesGroupShape(db, "name");
    ASSERT_TRUE (seriesGroupShape.configure({groove_shapes::SourceDescriptor(datasetUrl, "foo")}).isOk());
}

TEST_F(SeriesGroupShapeTest, GetSeriesGroupValue)
{
    groove_shapes::SeriesGroupShape seriesGroupShape(db, "name");
    ASSERT_TRUE (seriesGroupShape.configure({groove_shapes::SourceDescriptor(datasetUrl, "foo")}).isOk());
    auto result = seriesGroupShape.getSeriesGroupValue("test:/ foo foo1", 0.0);
    ASSERT_TRUE (result.isResult());
    auto datum = result.getResult();
    ASSERT_EQ (0.0, datum.key);
    ASSERT_EQ (4.0, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
}

TEST_F(SeriesGroupShapeTest, GetSeriesGroupValues)
{
    groove_shapes::SeriesGroupShape seriesGroupShape(db, "name");
    ASSERT_TRUE (seriesGroupShape.configure({groove_shapes::SourceDescriptor(datasetUrl, "foo")}).isOk());
    groove_data::DoubleRange range;
    range.start = Option<double>(0.0);
    range.start_exclusive = false;
    range.end = Option<double>(1.0);
    range.end_exclusive = false;
    auto result = seriesGroupShape.getSeriesGroupValues("test:/ foo foo1", range);
    ASSERT_TRUE (result.isResult());
    auto it = result.getResult();
    groove_shapes::SeriesGroupDatum datum;
    ASSERT_TRUE (it.getNext(datum));
    ASSERT_EQ (0.0, datum.key);
    ASSERT_EQ (4.0, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
    ASSERT_TRUE (it.getNext(datum));
    ASSERT_EQ (1.0, datum.key);
    ASSERT_EQ (5.0, datum.value);
    ASSERT_EQ (groove_data::DatumFidelity::FIDELITY_VALID, datum.fidelity);
    ASSERT_FALSE (it.getNext(datum));
}