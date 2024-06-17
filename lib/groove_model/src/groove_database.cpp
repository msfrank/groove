
#include <groove_model/column_traits.h>
#include <groove_model/groove_database.h>
#include <groove_model/indexed_column_writer_template.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <tempo_utils/logging.h>
#include <tempo_utils/tempdir_maker.h>

groove_model::GrooveDatabase::GrooveDatabase(const groove_model::DatabaseOptions &options)
    : m_options(options),
      m_databaseId(std::make_shared<const std::string>()),
      m_lock(new absl::Mutex())
{
}

groove_model::GrooveDatabase::GrooveDatabase(
    std::shared_ptr<const std::string> databaseId,
    const groove_model::DatabaseOptions &options)
    : m_options(options),
      m_databaseId(databaseId),
      m_lock(new absl::Mutex())
{
    TU_ASSERT (m_databaseId != nullptr && !m_databaseId->empty());
}
groove_model::GrooveDatabase::~GrooveDatabase()
{
    delete m_lock;
}

tempo_utils::Status
groove_model::GrooveDatabase::configure()
{
    absl::WriterMutexLock locker(m_lock);

    std::filesystem::path dbDirectory;
    if (m_options.modelsDirectory.empty()) {
        dbDirectory = std::filesystem::current_path();
    } else {
        dbDirectory = m_options.modelsDirectory;
    }
    if (m_databaseId->empty()) {
        tempo_utils::TempdirMaker tempdirMaker(dbDirectory, "models.XXXXXXXX");
        m_dbDirectory = tempdirMaker.getTempdir();
        m_databaseId = std::make_shared<const std::string>(m_dbDirectory.filename().string());
    } else {
        m_dbDirectory = dbDirectory / *m_databaseId;
    }

    auto store = groove_model::RocksDbStore::create(m_dbDirectory);
    auto status = store->open();
    if (!status.ok())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
    m_store = store;

    return ModelStatus::ok();
}

inline groove_data::CollationMode parse_model_key_collation(groove_model::ModelKeyCollation collation) {
    switch (collation) {
        case groove_model::ModelKeyCollation::Sorted:
            return groove_data::CollationMode::COLLATION_SORTED;
        case groove_model::ModelKeyCollation::Indexed:
            return groove_data::CollationMode::COLLATION_INDEXED;
        default:
            return groove_data::CollationMode::INVALID;
    }
}

inline groove_data::DataKeyType parse_model_key_type(groove_model::ModelKeyType key) {
    switch (key) {
        case groove_model::ModelKeyType::Double:
            return groove_data::DataKeyType::KEY_DOUBLE;
        case groove_model::ModelKeyType::Int64:
            return groove_data::DataKeyType::KEY_INT64;
        case groove_model::ModelKeyType::Category:
            return groove_data::DataKeyType::KEY_CATEGORY;
        default:
            return groove_data::DataKeyType::INVALID;
    }
}

inline groove_data::DataValueType parse_column_value_type(groove_model::ColumnValueType value) {
    switch (value) {
        case groove_model::ColumnValueType::Double:
            return groove_data::DataValueType::VALUE_TYPE_DOUBLE;
        case groove_model::ColumnValueType::Int64:
            return groove_data::DataValueType::VALUE_TYPE_INT64;
        case groove_model::ColumnValueType::String:
            return groove_data::DataValueType::VALUE_TYPE_STRING;
        default:
            return groove_data::DataValueType::INVALID;
    }
}

static std::shared_ptr<groove_model::GrooveModel>
create_model(
    const tempo_utils::Url &datasetUrl,
    const groove_model::ModelWalker &model,
    std::shared_ptr<groove_model::RocksDbStore> store)
{
    if (!model.isValid())
        return {};
    auto modelId = std::make_shared<const std::string>(model.getModelId());
    if (modelId->empty())
        return {};

    auto collation = parse_model_key_collation(model.getKeyCollation());
    auto keyType = parse_model_key_type(model.getKeyType());

    absl::flat_hash_map<std::string,groove_data::DataValueType> columns;
    for (int i = 0; i < model.numColumns(); i++) {
        auto column = model.getColumn(i);
        if (!column.isValid())
            continue;
        auto columnId = column.getColumnId();
        if (columnId.empty())
            continue;
        auto valueType = parse_column_value_type(column.getValueType());
        columns[columnId] = valueType;
    }

    return std::make_shared<groove_model::GrooveModel>(datasetUrl, modelId, collation, keyType, columns, store);
}

tempo_utils::Status
groove_model::GrooveDatabase::declareDataset(const tempo_utils::Url &datasetUrl, const GrooveSchema &schema)
{
    absl::WriterMutexLock locker(m_lock);

    if (m_datasets.contains(datasetUrl))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "dataset already exists");

    auto walker = schema.getSchema();

    absl::flat_hash_map<std::string,std::shared_ptr<GrooveModel>> models;
    for (tu_uint32 i = 0; i < walker.numModels(); i++) {
        auto model = create_model(datasetUrl, walker.getModel(i), m_store);
        if (model != nullptr) {
            models[*model->getModelId()] = model;
        }
    }
    auto dataset = std::make_shared<DatabaseDataset>(datasetUrl, schema, models);
    m_datasets[datasetUrl] = dataset;

    return ModelStatus::ok();
}

bool
groove_model::GrooveDatabase::hasDataset(const tempo_utils::Url &datasetUrl) const
{
    absl::ReaderMutexLock locker(m_lock);

    return m_datasets.contains(datasetUrl);
}

std::shared_ptr<groove_model::AbstractDataset>
groove_model::GrooveDatabase::getDataset(const tempo_utils::Url &datasetUrl) const
{
    absl::ReaderMutexLock locker(m_lock);

    if (m_datasets.contains(datasetUrl))
        return m_datasets.at(datasetUrl);
    return {};
}

groove_model::GrooveSchema
groove_model::GrooveDatabase::getSchema(const tempo_utils::Url &datasetUrl) const
{
    absl::ReaderMutexLock locker(m_lock);

    if (m_datasets.contains(datasetUrl))
        return m_datasets.at(datasetUrl)->getSchema();
    return {};
}

tempo_utils::Status
groove_model::GrooveDatabase::dropDataset(const tempo_utils::Url &datasetUrl)
{
    absl::WriterMutexLock locker(m_lock);
    m_datasets.erase(datasetUrl);
    return ModelStatus::ok();
}

template <typename DefType, typename VectorType>
static tempo_utils::Status
update_indexed_column(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::string &columnId,
    std::shared_ptr<groove_model::AbstractPageStore> pageStore,
    std::shared_ptr<VectorType> vector)
{
    auto writer = groove_model::IndexedColumnWriter<DefType>::create(
        datasetUrl,
        std::make_shared<const std::string>(modelId),
        std::make_shared<const std::string>(columnId),
        pageStore);

    return writer->setValues(vector);
}

static tempo_utils::Status
update_model_indexed_column(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    const std::string &columnId,
    std::shared_ptr<groove_model::AbstractPageStore> pageStore,
    std::shared_ptr<groove_data::BaseVector> vector)
{
    switch (vector->getVectorType()) {
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE:
            return update_indexed_column<groove_model::CategoryDouble,groove_data::CategoryDoubleVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::CategoryDoubleVector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_INT64:
            return update_indexed_column<groove_model::CategoryInt64,groove_data::CategoryInt64Vector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::CategoryInt64Vector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_STRING:
            return update_indexed_column<groove_model::CategoryString,groove_data::CategoryStringVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::CategoryStringVector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE:
            return update_indexed_column<groove_model::DoubleDouble,groove_data::DoubleDoubleVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::DoubleDoubleVector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_INT64:
            return update_indexed_column<groove_model::DoubleInt64,groove_data::DoubleInt64Vector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::DoubleInt64Vector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_STRING:
            return update_indexed_column<groove_model::DoubleString,groove_data::DoubleStringVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::DoubleStringVector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_DOUBLE:
            return update_indexed_column<groove_model::Int64Double,groove_data::Int64DoubleVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::Int64DoubleVector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_INT64:
            return update_indexed_column<groove_model::Int64Int64,groove_data::Int64Int64Vector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::Int64Int64Vector>(vector));
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_STRING:
            return update_indexed_column<groove_model::Int64String,groove_data::Int64StringVector>(
                datasetUrl, modelId, columnId, pageStore,
                std::static_pointer_cast<groove_data::Int64StringVector>(vector));
        default:
            return groove_model::ModelStatus::forCondition(
                groove_model::ModelCondition::kModelInvariant, "invalid vector type");
    }
}

tempo_utils::Status
groove_model::GrooveDatabase::updateModel(
    const tempo_utils::Url &datasetUrl,
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame,
    std::vector<std::string> *failedVectors)
{
    absl::WriterMutexLock locker(m_lock);

    if (!m_datasets.contains(datasetUrl))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "missing dataset");
    const auto dataset = m_datasets.at(datasetUrl);
    if (!dataset->hasModel(modelId))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "missing model");
    auto model = dataset->getModel(modelId);

    TU_LOG_INFO << "found model " << modelId;
    if (frame == nullptr)
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid frame");
    if (frame->getKeyType() != model->getKeyType())
        return ModelStatus::forCondition(ModelCondition::kModelInvariant, "frame has the wrong key type for model");

    for (auto iterator = frame->vectorsBegin(); iterator != frame->vectorsEnd(); iterator++) {
        auto columnId = iterator->first;
        TU_LOG_INFO << "updating column " << columnId << " for model " << modelId;
        auto vector = iterator->second;
        if (!model->hasColumn(columnId)) {
            TU_LOG_INFO << "model " << modelId << " is missing column " << columnId;
            if (failedVectors) {
                failedVectors->push_back(columnId);
            }
            continue;
        }
        auto columnDef = model->getColumnDef(columnId);
        if (columnDef.getValue() != vector->getValueType()) {
            TU_LOG_INFO << "model " << modelId << " is missing column " << columnId;
            if (failedVectors) {
                failedVectors->push_back(columnId);
            }
            continue;
        }
        switch (columnDef.getCollation()) {
            case groove_data::CollationMode::COLLATION_INDEXED:
                return update_model_indexed_column(datasetUrl, modelId, columnId, m_store, vector);
            case groove_data::CollationMode::COLLATION_SORTED:
            default:
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "invalid column collation");
        }
    }
    return ModelStatus::ok();
}

groove_model::DatabaseDataset::DatabaseDataset(
    const tempo_utils::Url &datasetUrl,
    const GrooveSchema &schema,
    const absl::flat_hash_map<std::string, std::shared_ptr<GrooveModel>> &models)
    : m_datasetUrl(datasetUrl),
      m_schema(schema),
      m_models(models)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_schema.isValid());
}

tempo_utils::Url
groove_model::DatabaseDataset::getDatasetUrl() const
{
    return m_datasetUrl;
}

groove_model::GrooveSchema
groove_model::DatabaseDataset::getSchema() const
{
    return m_schema;
}

bool
groove_model::DatabaseDataset::isImmutable() const
{
    return false;
}

bool
groove_model::DatabaseDataset::hasModel(const std::string &modelId) const
{
    return m_models.contains(modelId);
}

std::shared_ptr<groove_model::GrooveModel>
groove_model::DatabaseDataset::getModel(const std::string &modelId) const
{
    if (m_models.contains(modelId))
        return m_models.at(modelId);
    return {};
}