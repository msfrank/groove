
#include <groove_data/category_frame.h>
#include <groove_data/category_double_vector.h>
#include <groove_model/base_column.h>
#include <groove_model/column_traits.h>
#include <groove_model/groove_model.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>

groove_model::GrooveModel::GrooveModel(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    groove_data::CollationMode collation,
    groove_data::DataKeyType key,
    const absl::flat_hash_map<std::string,groove_data::DataValueType> &columns,
    std::shared_ptr<AbstractPageCache> pageCache)
    : m_datasetUrl(datasetUrl),
      m_modelId(modelId),
      m_collation(collation),
      m_key(key),
      m_pageCache(pageCache)
{
    TU_ASSERT (m_datasetUrl.isValid());
    TU_ASSERT (m_modelId != nullptr && !m_modelId->empty());
    TU_ASSERT (m_collation != groove_data::CollationMode::COLLATION_UNKNOWN);
    TU_ASSERT (m_key != groove_data::DataKeyType::KEY_UNKNOWN);
    TU_ASSERT (m_pageCache != nullptr);

    for (auto iterator = columns.cbegin(); iterator != columns.cend(); iterator++) {
        m_columns[iterator->first] = ColumnDef(m_collation, m_key, iterator->second);
    }
}

tempo_utils::Url
groove_model::GrooveModel::getDatasetUrl() const
{
    return m_datasetUrl;
}

std::shared_ptr<const std::string>
groove_model::GrooveModel::getModelId() const
{
    return m_modelId;
}

groove_data::CollationMode
groove_model::GrooveModel::getCollationMode() const
{
    return m_collation;
}

groove_data::DataKeyType
groove_model::GrooveModel::getKeyType()
{
    return m_key;
}

bool
groove_model::GrooveModel::hasColumn(const std::string &columnId) const
{
    return m_columns.contains(columnId);
}

bool
groove_model::GrooveModel::hasColumn(const std::string &columnId, groove_data::DataValueType valueType) const
{
    if (!m_columns.contains(columnId))
        return false;
    auto columnDef = m_columns.at(columnId);
    return columnDef.getCollation() == m_collation
           && columnDef.getKey() == m_key
           && columnDef.getValue() == valueType;
}

bool
groove_model::GrooveModel::hasColumn(const std::string &columnId, const ColumnDef &columnDef) const
{
    if (!m_columns.contains(columnId))
        return false;
    auto columnDef_ = m_columns.at(columnId);
    return columnDef_.getCollation() == columnDef.getCollation()
        && columnDef_.getKey() == columnDef.getKey()
        && columnDef_.getValue() == columnDef.getValue();
}

groove_model::ColumnDef
groove_model::GrooveModel::getColumnDef(const std::string &columnId) const
{
    if (!m_columns.contains(columnId))
        return {};
    return m_columns.at(columnId);
}

absl::flat_hash_map<std::string,groove_model::ColumnDef>::const_iterator
groove_model::GrooveModel::columnsBegin() const
{
    return m_columns.cbegin();
}

absl::flat_hash_map<std::string,groove_model::ColumnDef>::const_iterator
groove_model::GrooveModel::columnsEnd() const
{
    return m_columns.cend();
}

groove_model::ColumnDef::ColumnDef()
    : m_collation(groove_data::CollationMode::COLLATION_UNKNOWN),
      m_key(groove_data::DataKeyType::KEY_UNKNOWN),
      m_value(groove_data::DataValueType::VALUE_TYPE_UNKNOWN)
{
}

groove_model::ColumnDef::ColumnDef(
    groove_data::CollationMode collation,
    groove_data::DataKeyType key,
    groove_data::DataValueType value)
    : m_collation(collation),
      m_key(key),
      m_value(value)
{
}

bool
groove_model::ColumnDef::isValid() const
{
    return m_collation == groove_data::CollationMode::COLLATION_UNKNOWN
        || m_key == groove_data::DataKeyType::KEY_UNKNOWN
        || m_value == groove_data::DataValueType::VALUE_TYPE_UNKNOWN;
}

groove_model::ColumnDef::ColumnDef(const ColumnDef &other)
    : m_collation(other.m_collation),
      m_key(other.m_key),
      m_value(other.m_value)
{
}

groove_data::CollationMode
groove_model::ColumnDef::getCollation() const
{
    return m_collation;
}

groove_data::DataKeyType
groove_model::ColumnDef::getKey() const
{
    return m_key;
}

groove_data::DataValueType
groove_model::ColumnDef::getValue() const
{
    return m_value;
}
