#ifndef GROOVE_MODEL_SCHEMA_WALKER_H
#define GROOVE_MODEL_SCHEMA_WALKER_H

#include <tempo_utils/integer_types.h>

#include "model_types.h"
#include "model_walker.h"
#include "namespace_walker.h"

namespace groove_model {

    class SchemaWalker {

    public:
        SchemaWalker();
        SchemaWalker(const SchemaWalker &other);

        bool isValid() const;

        NamespaceWalker getNamespace(tu_uint32 index) const;
        int numNamespaces() const;

        ModelWalker getModel(tu_uint32 index) const;
        ModelWalker findModel(const std::string &modelId) const;
        tu_uint32 numModels() const;

    private:
        std::shared_ptr<const internal::SchemaReader> m_reader;

        explicit SchemaWalker(std::shared_ptr<const internal::SchemaReader> reader);
        friend class GrooveSchema;
    };
}

#endif // GROOVE_MODEL_SCHEMA_WALKER_H
