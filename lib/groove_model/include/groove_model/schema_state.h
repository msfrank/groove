#ifndef GROOVE_MODEL_SCHEMA_STATE_H
#define GROOVE_MODEL_SCHEMA_STATE_H

#include <string>

#include <absl/strings/string_view.h>

#include <tempo_utils/url.h>

#include "groove_schema.h"
#include "model_result.h"
#include "schema_attr_writer.h"

namespace groove_model {

    // forward declarations
    class SchemaAttr;
    class SchemaColumn;
    class SchemaModel;
    class SchemaNamespace;

    class SchemaState {

    public:
        SchemaState();
        ~SchemaState();

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        SchemaNamespace *getNamespace(int index) const;
        SchemaNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        tempo_utils::Result<SchemaNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        std::vector<SchemaNamespace *>::const_iterator namespacesBegin() const;
        std::vector<SchemaNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        SchemaAttr *getAttr(int index) const;
        std::vector<SchemaAttr *>::const_iterator attrsBegin() const;
        std::vector<SchemaAttr *>::const_iterator attrsEnd() const;
        int numAttrs() const;

        tempo_utils::Result<SchemaModel *> putModel(
            std::string_view modelId,
            ModelKeyType keyType,
            ModelKeyCollation keyCollation);
        SchemaModel *getModel(int index) const;
        std::vector<SchemaModel *>::const_iterator modelsBegin() const;
        std::vector<SchemaModel *>::const_iterator modelsEnd() const;
        int numModels() const;

        tempo_utils::Result<SchemaColumn *> appendColumn(
            std::string_view columnId,
            ColumnValueType valueType,
            ColumnValueFidelity valueFidelity);
        SchemaColumn *getColumn(int index) const;
        std::vector<SchemaColumn *>::const_iterator columnsBegin() const;
        std::vector<SchemaColumn *>::const_iterator columnsEnd() const;
        int numColumns() const;

        tempo_utils::Result<GrooveSchema> toSchema(bool noIdentifier = false) const;

    private:
        std::vector<SchemaNamespace *> m_schemaNamespaces;
        std::vector<SchemaAttr *> m_schemaAttrs;
        std::vector<SchemaColumn *> m_schemaColumns;
        std::vector<SchemaModel *> m_schemaModels;
        absl::flat_hash_map<tempo_utils::Url,SchemaNamespace *> m_namespaceIndex;
        absl::flat_hash_map<std::string,SchemaModel *> m_modelIndex;

        tempo_utils::Result<tu_uint32> putNamespace(const char *nsString);
        tempo_utils::Result<SchemaAttr *> appendAttr(AttrId id, tempo_utils::AttrValue value);

        friend class SchemaAttrWriter;

    public:
        /**
          *
          * @tparam T
          * @param serde
          * @param value
          * @return
          */
        template <typename T>
        tempo_utils::Result<SchemaAttr *>
        appendAttr(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            SchemaAttrWriter writer(serde.getKey(), this);
            auto result = serde.writeAttr(&writer, value);
            if (result.isStatus())
                return result.getStatus();
            auto *attr = getAttr(result.getResult());
            if (attr == nullptr)
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "missing serialized attr");
            return attr;
        };

        template <typename T>
        SchemaAttr *
        appendAttrOrThrow(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            auto appendAttrResult = appendAttr(serde, value);
            if (appendAttrResult.isResult())
                return appendAttrResult.getResult();
            throw tempo_utils::StatusException(appendAttrResult.getStatus());
        };
    };
}

#endif // GROOVE_MODEL_SCHEMA_STATE_H