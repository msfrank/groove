#ifndef GROOVE_MODEL_INTERNAL_SCHEMA_READER_H
#define GROOVE_MODEL_INTERNAL_SCHEMA_READER_H

#include <groove_model/generated/schema.h>
#include <tempo_utils/integer_types.h>

namespace groove_model::internal {

    class SchemaReader {

    public:
        SchemaReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        gms1::SchemaVersion getABI() const;

        const gms1::NamespaceDescriptor *getNamespace(tu_uint32 index) const;
        tu_uint32 numNamespaces() const;

        const gms1::AttrDescriptor *getAttr(tu_uint32 index) const;
        tu_uint32 numAttrs() const;

        const gms1::ColumnDescriptor *getColumn(tu_uint32 index) const;
        tu_uint32 numColumns() const;

        const gms1::ModelDescriptor *getModel(tu_uint32 index) const;
        const gms1::ModelDescriptor *findModel(const std::string &modelId) const;
        tu_uint32 numModels() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const gms1::Schema *m_schema;
    };
}

#endif // GROOVE_MODEL_INTERNAL_SCHEMA_READER_H
