
#include <groove_model/internal/schema_reader.h>
#include <groove_model/column_walker.h>

groove_model::ColumnWalker::ColumnWalker()
    : m_columnDescriptor(nullptr)
{
}

groove_model::ColumnWalker::ColumnWalker(
    std::shared_ptr<const internal::SchemaReader> reader,
    void *columnDescriptor)
    : m_reader(reader),
      m_columnDescriptor(columnDescriptor)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_columnDescriptor != nullptr);
}

groove_model::ColumnWalker::ColumnWalker(const ColumnWalker &other)
    : m_reader(other.m_reader),
      m_columnDescriptor(other.m_columnDescriptor)
{
}

bool
groove_model::ColumnWalker::isValid() const
{
    return m_reader != nullptr;
}

std::string
groove_model::ColumnWalker::getColumnId() const
{
    if (!isValid())
        return {};
    auto *columnDescriptor = static_cast<const gms1::ColumnDescriptor *>(m_columnDescriptor);
    if (columnDescriptor->id() == nullptr)
        return {};
    return columnDescriptor->id()->str();
}

groove_model::ColumnValueType
groove_model::ColumnWalker::getValueType() const
{
    if (!isValid())
        return {};
    auto *columnDescriptor = static_cast<const gms1::ColumnDescriptor *>(m_columnDescriptor);
    switch (columnDescriptor->value_type()) {
        case gms1::ValueType::Double:
            return ColumnValueType::Double;
        case gms1::ValueType::Int64:
            return ColumnValueType::Int64;
        case gms1::ValueType::String:
            return ColumnValueType::String;
        default:
            return ColumnValueType::Invalid;
    }
}

groove_model::ColumnValueFidelity
groove_model::ColumnWalker::getValueFidelity() const
{
    if (!isValid())
        return {};
    auto *columnDescriptor = static_cast<const gms1::ColumnDescriptor *>(m_columnDescriptor);
    switch (columnDescriptor->fidelity()) {
        case gms1::ValueFidelity::OnlyValidValue:
            return ColumnValueFidelity::OnlyValidValue;
        case gms1::ValueFidelity::OnlyValidOrEmpty:
            return ColumnValueFidelity::OnlyValidOrEmpty;
        case gms1::ValueFidelity::AnyFidelityAllowed:
            return ColumnValueFidelity::AnyFidelityAllowed;
        default:
            return ColumnValueFidelity::Invalid;
    }
}

tu_uint32
groove_model::ColumnWalker::findIndexForAttr(const tempo_utils::AttrKey &key) const
{
    if (!isValid())
        return kInvalidOffsetU32;
    auto *columnDescriptor = static_cast<const gms1::ColumnDescriptor *>(m_columnDescriptor);
    auto *attrs = columnDescriptor->attrs();
    if (attrs == nullptr)    // node has no attrs
        return kInvalidOffsetU32;
    for (const auto attrIndex : *attrs) {
        auto *attr = m_reader->getAttr(attrIndex);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->id())
            return attrIndex;
    }
    return kInvalidOffsetU32;
}
