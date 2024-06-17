
#include <groove_model/internal/schema_reader.h>
#include <groove_model/namespace_walker.h>

groove_model::NamespaceWalker::NamespaceWalker()
    : m_reader(),
      m_index(0)
{
}

groove_model::NamespaceWalker::NamespaceWalker(std::shared_ptr<const internal::SchemaReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

groove_model::NamespaceWalker::NamespaceWalker(const NamespaceWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
groove_model::NamespaceWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numNamespaces();
}

tu_uint32
groove_model::NamespaceWalker::getIndex() const
{
    return m_index;
}

std::string_view
groove_model::NamespaceWalker::urlView() const
{
    if (!isValid())
        return {};
    auto *ns = m_reader->getNamespace(m_index);
    if (ns == nullptr || ns->ns_url() == nullptr)
        return {};
    return ns->ns_url()->string_view();
}

tempo_utils::Url
groove_model::NamespaceWalker::getUrl() const
{
    return tempo_utils::Url::fromString(urlView());
}
