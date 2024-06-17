
#include <groove_model/model_types.h>

groove_model::DataDef::DataDef()
    : m_key(groove_data::DataKeyType::KEY_UNKNOWN),
      m_value(groove_data::DataValueType::VALUE_TYPE_UNKNOWN)
{
}

groove_model::DataDef::DataDef(
    groove_data::DataKeyType key,
    groove_data::DataValueType value)
        : m_key(key),
          m_value(value)
{
}

groove_model::DataDef::DataDef(const DataDef &other)
    : m_key(other.m_key),
      m_value(other.m_value)
{
}

groove_data::DataKeyType
groove_model::DataDef::getKey() const
{
    return m_key;
}

groove_data::DataValueType
groove_model::DataDef::getValue() const
{
    return m_value;
}

groove_model::CategoryDouble::CategoryDouble()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::CategoryInt64::CategoryInt64()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::CategoryString::CategoryString()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::DoubleDouble::DoubleDouble()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::DoubleInt64::DoubleInt64()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::DoubleString::DoubleString()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::Int64Double::Int64Double()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::Int64Int64::Int64Int64()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::Int64String::Int64String()
    : DataDef(static_key_type(), static_value_type())
{
}

groove_model::AttrId::AttrId()
    : m_address(),
      m_type(kInvalidOffsetU32)
{
}

groove_model::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != kInvalidOffsetU32);
}

groove_model::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

groove_model::NamespaceAddress
groove_model::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
groove_model::AttrId::getType() const
{
    return m_type;
}

bool
groove_model::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}
