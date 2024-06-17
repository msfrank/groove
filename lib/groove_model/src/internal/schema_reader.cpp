
#include <flatbuffers/idl.h>

#include <groove_model/generated/schema_schema.h>
#include <groove_model/internal/schema_reader.h>
#include <tempo_utils/log_stream.h>

groove_model::internal::SchemaReader::SchemaReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_schema = gms1::GetSchema(m_bytes.data());
}

bool
groove_model::internal::SchemaReader::isValid() const
{
    return m_schema != nullptr;
}

gms1::SchemaVersion
groove_model::internal::SchemaReader::getABI() const
{
    if (m_schema == nullptr)
        return gms1::SchemaVersion::Unknown;
    return m_schema->abi();
}

const gms1::NamespaceDescriptor *
groove_model::internal::SchemaReader::getNamespace(tu_uint32 index) const
{
    if (m_schema == nullptr)
        return nullptr;
    if (m_schema->namespaces() && index < m_schema->namespaces()->size())
        return m_schema->namespaces()->Get(index);
    return nullptr;
}

tu_uint32
groove_model::internal::SchemaReader::numNamespaces() const
{
    if (m_schema == nullptr)
        return 0;
    return m_schema->namespaces()? m_schema->namespaces()->size() : 0;
}

const gms1::AttrDescriptor *
groove_model::internal::SchemaReader::getAttr(tu_uint32 index) const
{
    if (m_schema == nullptr)
        return nullptr;
    if (m_schema->attrs() && index < m_schema->attrs()->size())
        return m_schema->attrs()->Get(index);
    return nullptr;
}

tu_uint32
groove_model::internal::SchemaReader::numAttrs() const
{
    if (m_schema == nullptr)
        return 0;
    return m_schema->attrs()? m_schema->attrs()->size() : 0;
}

const gms1::ColumnDescriptor *
groove_model::internal::SchemaReader::getColumn(tu_uint32 index) const
{
    if (m_schema == nullptr)
        return nullptr;
    if (m_schema->columns() && index < m_schema->columns()->size())
        return m_schema->columns()->Get(index);
    return nullptr;
}

tu_uint32
groove_model::internal::SchemaReader::numColumns() const
{
    if (m_schema == nullptr)
        return 0;
    return m_schema->columns()? m_schema->columns()->size() : 0;
}

const gms1::ModelDescriptor *
groove_model::internal::SchemaReader::getModel(tu_uint32 index) const
{
    if (m_schema == nullptr)
        return nullptr;
    if (m_schema->models() && index < m_schema->models()->size())
        return m_schema->models()->Get(index);
    return nullptr;
}

const gms1::ModelDescriptor *
groove_model::internal::SchemaReader::findModel(const std::string &modelId) const
{
    if (m_schema == nullptr)
        return nullptr;
    return m_schema->models() ? m_schema->models()->LookupByKey(modelId.c_str()) : nullptr;
}

tu_uint32
groove_model::internal::SchemaReader::numModels() const
{
    if (m_schema == nullptr)
        return 0;
    return m_schema->models()? m_schema->models()->size() : 0;
}

std::span<const tu_uint8>
groove_model::internal::SchemaReader::bytesView() const
{
    return m_bytes;
}

std::string
groove_model::internal::SchemaReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) groove_model::schema::schema::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
