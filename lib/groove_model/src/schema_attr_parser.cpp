
#include <groove_model/generated/schema.h>
#include <groove_model/internal/schema_reader.h>
#include <groove_model/schema_attr_parser.h>

groove_model::SchemaAttrParser::SchemaAttrParser(
    std::shared_ptr<const internal::SchemaReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

tempo_utils::Status
groove_model::SchemaAttrParser::getNil(tu_uint32 index, std::nullptr_t &nil)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::TrueFalseNilValue)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_TrueFalseNilValue();
    if (value->tfn() != gms1::TrueFalseNil::Nil)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    nil = nullptr;
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getBool(tu_uint32 index, bool &b)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::TrueFalseNilValue)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_TrueFalseNilValue();
    switch (value->tfn()) {
        case gms1::TrueFalseNil::True:
            b = true;
            return tempo_utils::AttrStatus::ok();
        case gms1::TrueFalseNil::False:
            b = false;
            return tempo_utils::AttrStatus::ok();
        case gms1::TrueFalseNil::Nil:
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    }
}

tempo_utils::Status
groove_model::SchemaAttrParser::getInt64(tu_uint32 index, tu_int64 &i64)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::Int64Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_Int64Value();
    i64 = value->i64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getFloat64(tu_uint32 index, double &dbl)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::Float64Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_Float64Value();
    dbl = value->f64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::UInt64Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_UInt64Value();
    u64 = value->u64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::UInt32Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_UInt32Value();
    u32 = value->u32();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::UInt16Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_UInt16Value();
    u16 = value->u16();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::UInt8Value)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_UInt8Value();
    u8 = value->u8();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
groove_model::SchemaAttrParser::getString(tu_uint32 index, std::string &str)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->value_type() != gms1::Value::StringValue)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->value_as_StringValue();
    str = value->utf8()? value->utf8()->str() : std::string();
    return tempo_utils::AttrStatus::ok();
}
