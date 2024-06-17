
#include <groove_model/schema_attr.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>

groove_model::SchemaModel::SchemaModel(
    const std::string &modelId,
    ModelKeyType keyType,
    ModelKeyCollation keyCollation,
    ModelAddress address,
    SchemaState *state)
    : m_modelId(modelId),
      m_keyType(keyType),
      m_keyCollation(keyCollation),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (!m_modelId.empty());
    TU_ASSERT (m_keyType != ModelKeyType::Invalid);
    TU_ASSERT (m_keyCollation != ModelKeyCollation::Invalid);
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

std::string
groove_model::SchemaModel::getModelId() const
{
    return m_modelId;
}

groove_model::ModelKeyType
groove_model::SchemaModel::getKeyType() const
{
    return m_keyType;
}

groove_model::ModelKeyCollation
groove_model::SchemaModel::getKeyCollation() const
{
    return m_keyCollation;
}

groove_model::ModelAddress
groove_model::SchemaModel::getAddress() const
{
    return m_address;
}

bool
groove_model::SchemaModel::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

groove_model::AttrAddress
groove_model::SchemaModel::getAttr(const AttrId &attrId) const
{
    if (m_attrs.contains(attrId))
        return m_attrs.at(attrId);
    return {};
}

tempo_utils::Status
groove_model::SchemaModel::putAttr(SchemaAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant,
            "model {} contains duplicate attr", m_modelId);
    m_attrs[attrId] = attr->getAddress();
    return {};
}

absl::flat_hash_map<groove_model::AttrId,groove_model::AttrAddress>::const_iterator
groove_model::SchemaModel::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<groove_model::AttrId,groove_model::AttrAddress>::const_iterator
groove_model::SchemaModel::attrsEnd() const
{
    return m_attrs.cend();
}

int
groove_model::SchemaModel::numAttrs() const
{
    return m_attrs.size();
}

bool
groove_model::SchemaModel::hasColumn(std::string_view columnId) const
{
    return m_columnIndex.contains(columnId);
}

groove_model::ColumnAddress
groove_model::SchemaModel::getColumn(int index) const
{
    if (0 <= index && index < m_columns.size())
        return m_columns.at(index);
    return {};
}

tempo_utils::Status
groove_model::SchemaModel::appendColumn(SchemaColumn *column)
{
    TU_ASSERT (column != nullptr);

    auto columnId = column->getColumnId();
    if (m_columnIndex.contains(columnId))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant,
            "model {} contains duplicate column", m_modelId);
    auto index = m_columns.size();
    m_columnIndex[columnId] = index;
    m_columns.push_back(column->getAddress());
    return {};
}

std::vector<groove_model::ColumnAddress>::const_iterator
groove_model::SchemaModel::columnsBegin() const
{
    return m_columns.cbegin();
}

std::vector<groove_model::ColumnAddress>::const_iterator
groove_model::SchemaModel::columnsEnd() const
{
    return m_columns.cend();
}

int
groove_model::SchemaModel::numColumns() const
{
    return m_columns.size();
}
