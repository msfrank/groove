#ifndef GROOVE_MODEL_SCHEMA_COLUMN_H
#define GROOVE_MODEL_SCHEMA_COLUMN_H

#include "model_types.h"
#include "schema_state.h"

namespace groove_model {

    class SchemaColumn {

    public:
        SchemaColumn(
            const std::string &columnId,
            ColumnValueType valueType,
            ColumnValueFidelity valueFidelity,
            ColumnAddress address,
            SchemaState *state);

        ColumnAddress getAddress() const;

        std::string getColumnId() const;
        ColumnValueType getValueType() const;
        ColumnValueFidelity getValueFidelity() const;

        bool hasAttr(const AttrId &attrId) const;
        AttrAddress getAttr(const AttrId &attrId) const;
        tempo_utils::Status putAttr(SchemaAttr *attr);
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsEnd() const;
        int numAttrs() const;

    private:
        std::string m_columnId;
        ColumnValueType m_valueType;
        ColumnValueFidelity m_valueFidelity;
        ColumnAddress m_address;
        SchemaState *m_state;
        absl::flat_hash_map<AttrId,AttrAddress> m_attrs;
    };
}

#endif // GROOVE_MODEL_SCHEMA_COLUMN_H
