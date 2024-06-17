
#include <groove_io/dataset_file.h>

groove_io::DatasetFile::DatasetFile(
    tempo_utils::Url datasetUrl,
    const absl::flat_hash_map<std::string, std::shared_ptr<groove_model::GrooveModel>> &models,
    std::shared_ptr<DatasetReader> reader)
    : m_datasetUrl(datasetUrl),
      m_models(models),
      m_reader(reader)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_reader != nullptr);
}

bool
groove_io::DatasetFile::isValid() const
{
    if (m_reader != nullptr)
        return m_reader->isValid();
    return false;
}

std::filesystem::path
groove_io::DatasetFile::getDatasetPath() const
{
    if (m_reader != nullptr)
        return m_reader->getDatasetPath();
    return {};
}

tempo_utils::Url
groove_io::DatasetFile::getDatasetUrl() const
{
    return m_datasetUrl;
}

groove_model::GrooveSchema
groove_io::DatasetFile::getSchema() const
{
    if (m_reader != nullptr)
        return m_reader->getSchema();
    return {};
}

bool
groove_io::DatasetFile::isImmutable() const
{
    return true;
}

bool
groove_io::DatasetFile::hasModel(const std::string &modelId) const
{
    return m_models.contains(modelId);
}

std::shared_ptr<groove_model::GrooveModel>
groove_io::DatasetFile::getModel(const std::string &modelId) const
{
    if (m_models.contains(modelId))
        return m_models.at(modelId);
    return {};
}

inline groove_data::CollationMode parse_gms1_key_collation(groove_model::ModelKeyCollation collation) {
    switch (collation) {
        case groove_model::ModelKeyCollation::Sorted:
            return groove_data::CollationMode::COLLATION_SORTED;
        case groove_model::ModelKeyCollation::Indexed:
            return groove_data::CollationMode::COLLATION_INDEXED;
        default:
            return groove_data::CollationMode::INVALID;
    }
}

inline groove_data::DataKeyType parse_gms1_key_type(groove_model::ModelKeyType key) {
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

inline groove_data::DataValueType parse_gms1_value_type(groove_model::ColumnValueType value) {
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
    groove_model::ModelWalker model,
    std::shared_ptr<groove_io::DatasetReader> reader)
{
    if (!model.isValid())
        return {};
    auto modelId = std::make_shared<const std::string>(model.getModelId());
    if (modelId->empty())
        return {};

    auto keyCollation = parse_gms1_key_collation(model.getKeyCollation());
    auto keyType = parse_gms1_key_type(model.getKeyType());

    absl::flat_hash_map<std::string,groove_data::DataValueType> columns;
    for (int i = 0; i < model.numColumns(); i++) {
        auto column = model.getColumn(i);
        if (!column.isValid())
            continue;
        auto columnId = column.getColumnId();
        if (columnId.empty())
            continue;
        auto valueType = parse_gms1_value_type(column.getValueType());
        columns[columnId] = valueType;
    }

    return std::make_shared<groove_model::GrooveModel>(datasetUrl, modelId, keyCollation, keyType, columns, reader);
}

tempo_utils::Result<std::shared_ptr<groove_io::DatasetFile>>
groove_io::DatasetFile::create(
    const std::filesystem::path &path,
    const tempo_utils::Url &overrideUrl)
{
    auto createReaderResult = DatasetReader::create(path);
    if (createReaderResult.isStatus())
        return createReaderResult.getStatus();
    auto reader = createReaderResult.getResult();

    // determine the dataset url to use
    tempo_utils::Url datasetUrl;
    if (!overrideUrl.isValid()) {
        datasetUrl = tempo_utils::Url::fromString(
            absl::StrCat("file://", absolute(path).string()));
    } else {
        datasetUrl = overrideUrl;
    }

    // create models
    auto schema = reader->getSchema().getSchema();
    absl::flat_hash_map<std::string, std::shared_ptr<groove_model::GrooveModel>> models;
    for (int i = 0; i < schema.numModels(); i++) {
        auto model = create_model(datasetUrl, schema.getModel(i), reader);
        if (model != nullptr) {
            models[*model->getModelId()] = model;
        }
    }

    return std::shared_ptr<DatasetFile>(new DatasetFile(datasetUrl, models, reader));
}