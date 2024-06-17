#ifndef GROOVE_MODEL_NAMESPACE_WALKER_H
#define GROOVE_MODEL_NAMESPACE_WALKER_H

#include <tempo_utils/url.h>

#include "model_types.h"

namespace groove_model {

    class NamespaceWalker {

    public:
        NamespaceWalker();
        NamespaceWalker(const NamespaceWalker &other);

        bool isValid() const;

        tu_uint32 getIndex() const;

        tempo_utils::Url getUrl() const;
        std::string_view urlView() const;

    private:
        std::shared_ptr<const internal::SchemaReader> m_reader;
        tu_uint32 m_index;

        NamespaceWalker(std::shared_ptr<const internal::SchemaReader> reader, tu_uint32 index);
        friend class SchemaWalker;
        friend class ValueWalker;
    };
}

#endif // GROOVE_MODEL_NAMESPACE_WALKER_H
