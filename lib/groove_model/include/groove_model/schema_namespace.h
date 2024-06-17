#ifndef GROOVE_MODEL_SCHEMA_NAMESPACE_H
#define GROOVE_MODEL_SCHEMA_NAMESPACE_H

#include <tempo_utils/url.h>

#include "schema_state.h"

namespace groove_model {

    class SchemaNamespace {

    public:
        SchemaNamespace(
            const tempo_utils::Url &nsUrl,
            NamespaceAddress address,
            SchemaState *state);

        tempo_utils::Url getNsUrl() const;
        NamespaceAddress getAddress() const;

    private:
        tempo_utils::Url m_nsUrl;
        NamespaceAddress m_address;
        SchemaState *m_state;
    };
}

#endif // GROOVE_MODEL_SCHEMA_NAMESPACE_H
