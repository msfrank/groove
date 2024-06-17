
#include <groove_model/schema_attr.h>

groove_model::SchemaAttr::SchemaAttr(
    groove_model::AttrId id,
    tempo_utils::AttrValue value,
    groove_model::AttrAddress address,
    groove_model::SchemaState *state)
    : m_id(id),
      m_value(value),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

groove_model::AttrId
groove_model::SchemaAttr::getAttrId() const
{
    return m_id;
}

tempo_utils::AttrValue
groove_model::SchemaAttr::getAttrValue() const
{
    return m_value;
}

groove_model::AttrAddress
groove_model::SchemaAttr::getAddress() const
{
    return m_address;
}
