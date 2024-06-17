
#include <groove_model/schema_attr.h>
#include <groove_model/schema_column.h>

groove_model::SchemaColumn::SchemaColumn(
    const std::string &columnId,
    ColumnValueType valueType,
    ColumnValueFidelity valueFidelity,
    ColumnAddress address,
    SchemaState *state)
    : m_columnId(columnId),
      m_valueType(valueType),
      m_valueFidelity(valueFidelity),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (!m_columnId.empty());
    TU_ASSERT (m_valueType != ColumnValueType::Invalid);
    TU_ASSERT (m_valueFidelity != ColumnValueFidelity::Invalid);
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

std::string
groove_model::SchemaColumn::getColumnId() const
{
    return m_columnId;
}

groove_model::ColumnValueType
groove_model::SchemaColumn::getValueType() const
{
    return m_valueType;
}

groove_model::ColumnValueFidelity
groove_model::SchemaColumn::getValueFidelity() const
{
    return m_valueFidelity;
}

groove_model::ColumnAddress
groove_model::SchemaColumn::getAddress() const
{
    return m_address;
}

bool
groove_model::SchemaColumn::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

groove_model::AttrAddress
groove_model::SchemaColumn::getAttr(const AttrId &attrId) const
{
    if (m_attrs.contains(attrId))
        return m_attrs.at(attrId);
    return {};
}

tempo_utils::Status
groove_model::SchemaColumn::putAttr(SchemaAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant,
            "column {} contains duplicate attr", m_columnId);
    m_attrs[attrId] = attr->getAddress();
    return {};
}

absl::flat_hash_map<groove_model::AttrId,groove_model::AttrAddress>::const_iterator
groove_model::SchemaColumn::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<groove_model::AttrId,groove_model::AttrAddress>::const_iterator
groove_model::SchemaColumn::attrsEnd() const
{
    return m_attrs.cend();
}

int
groove_model::SchemaColumn::numAttrs() const
{
    return m_attrs.size();
}
