
#include <groove_model/schema_namespace.h>

groove_model::SchemaNamespace::SchemaNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    SchemaState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
groove_model::SchemaNamespace::getNsUrl() const
{
    return m_nsUrl;
}

groove_model::NamespaceAddress
groove_model::SchemaNamespace::getAddress() const
{
    return m_address;
}
