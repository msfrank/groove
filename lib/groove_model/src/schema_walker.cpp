
#include <groove_model/internal/schema_reader.h>
#include <groove_model/namespace_walker.h>
#include <groove_model/schema_walker.h>

groove_model::SchemaWalker::SchemaWalker()
{
}

groove_model::SchemaWalker::SchemaWalker(std::shared_ptr<const internal::SchemaReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

groove_model::SchemaWalker::SchemaWalker(const SchemaWalker &other)
    : m_reader(other.m_reader)
{
}

bool
groove_model::SchemaWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_model::NamespaceWalker
groove_model::SchemaWalker::getNamespace(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numNamespaces() <= index)
        return {};
    return NamespaceWalker(m_reader, index);
}

int
groove_model::SchemaWalker::numNamespaces() const
{
    if (!isValid())
        return {};
    return m_reader->numNamespaces();
}

//const gms1::ValueDescriptor *
//groove_model::GrooveSchema::getValue(tu_uint32 index) const
//{
//    if (m_schema == nullptr)
//        return nullptr;
//    if (m_schema->values() && index < m_schema->values()->size())
//        return m_schema->values()->Get(index);
//    return nullptr;
//}
//
//tu_uint32
//groove_model::GrooveSchema::numValues() const
//{
//    if (m_schema == nullptr)
//        return 0;
//    return m_schema->values()? m_schema->values()->size() : 0;
//}

groove_model::ModelWalker
groove_model::SchemaWalker::getModel(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = m_reader->getModel(index);
    if (modelDescriptor == nullptr)
        return {};
    return ModelWalker(m_reader, (void *) modelDescriptor);
}

groove_model::ModelWalker
groove_model::SchemaWalker::findModel(const std::string &modelId) const
{
    if (!isValid())
        return {};
    auto *modelDescriptor = m_reader->findModel(modelId);
    if (modelDescriptor == nullptr)
        return {};
    return ModelWalker(m_reader, (void *) modelDescriptor);
}

tu_uint32
groove_model::SchemaWalker::numModels() const
{
    if (!isValid())
        return 0;
    return m_reader->numModels();
}

//std::shared_ptr<const std::string>
//groove_model::GrooveSchema::getBytes() const
//{
//    return m_bytes;
//}
//
//bool
//groove_model::GrooveSchema::verify(const tu_uint8 *data, size_t size)
//{
//    flatbuffers::Verifier verifier(data, size);
//    return gms1::VerifySchemaBuffer(verifier);
//}
