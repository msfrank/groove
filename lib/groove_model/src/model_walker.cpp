
#include <groove_model/internal/schema_reader.h>
#include <groove_model/model_walker.h>

groove_model::ModelWalker::ModelWalker()
    : m_modelDescriptor(nullptr)
{
}

groove_model::ModelWalker::ModelWalker(
    std::shared_ptr<const internal::SchemaReader> reader,
    void *modelDescriptor)
    : m_reader(reader),
      m_modelDescriptor(modelDescriptor)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_modelDescriptor != nullptr);
}

groove_model::ModelWalker::ModelWalker(const ModelWalker &other)
    : m_reader(other.m_reader),
      m_modelDescriptor(other.m_modelDescriptor)
{
}

bool
groove_model::ModelWalker::isValid() const
{
    return m_reader != nullptr;
}

std::string
groove_model::ModelWalker::getModelId() const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    if (modelDescriptor->id() == nullptr)
        return {};
    return modelDescriptor->id()->str();
}

groove_model::ModelKeyType
groove_model::ModelWalker::getKeyType() const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    switch (modelDescriptor->key_type()) {
        case gms1::KeyType::Category:
            return ModelKeyType::Category;
        case gms1::KeyType::Double:
            return ModelKeyType::Double;
        case gms1::KeyType::Int64:
            return ModelKeyType::Int64;
        default:
            return ModelKeyType::Invalid;
    }
}

groove_model::ModelKeyCollation
groove_model::ModelWalker::getKeyCollation() const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    switch (modelDescriptor->collation()) {
        case gms1::KeyCollation::Sorted:
            return ModelKeyCollation::Sorted;
        case gms1::KeyCollation::Indexed:
            return ModelKeyCollation::Indexed;
        default:
            return ModelKeyCollation::Invalid;
    }
}

groove_model::ColumnWalker
groove_model::ModelWalker::getColumn(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    if (modelDescriptor->columns() == nullptr || modelDescriptor->columns()->size() <= index)
        return {};
    auto *columnDescriptor = m_reader->getColumn(modelDescriptor->columns()->Get(index));
    return ColumnWalker(m_reader, (void *) columnDescriptor);
}

groove_model::ColumnWalker
groove_model::ModelWalker::findColumn(const std::string &columnId) const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    if (modelDescriptor->columns() == nullptr)
        return {};
    for (tu_uint32 index : *modelDescriptor->columns()) {
        auto *columnDescriptor = m_reader->getColumn(index);
        TU_ASSERT (columnDescriptor != nullptr);
        if (columnDescriptor->id() == nullptr)
            continue;
        if (columnDescriptor->id()->string_view() == columnId)
            return ColumnWalker(m_reader, (void *) columnDescriptor);
    }
    return {};
}

int
groove_model::ModelWalker::numColumns() const
{
    if (!isValid())
        return 0;
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    if (modelDescriptor->columns() == nullptr)
        return 0;
    return modelDescriptor->columns()->size();
}

tu_uint32
groove_model::ModelWalker::findIndexForAttr(const tempo_utils::AttrKey &key) const
{
    if (!isValid())
        return kInvalidOffsetU32;
    auto *modelDescriptor = static_cast<const gms1::ModelDescriptor *>(m_modelDescriptor);
    auto *attrs = modelDescriptor->attrs();
    if (attrs == nullptr)    // model has no attrs
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
