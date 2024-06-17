#ifndef GROOVE_MODEL_SCHEMA_MODEL_H
#define GROOVE_MODEL_SCHEMA_MODEL_H

#include "schema_state.h"

namespace groove_model {

    class SchemaModel {

    public:
        SchemaModel(
            const std::string &modelId,
            ModelKeyType keyType,
            ModelKeyCollation keyCollation,
            ModelAddress address,
            SchemaState *state);

        ModelAddress getAddress() const;

        std::string getModelId() const;
        ModelKeyType getKeyType() const;
        ModelKeyCollation getKeyCollation() const;

        bool hasAttr(const AttrId &attrId) const;
        AttrAddress getAttr(const AttrId &attrId) const;
        tempo_utils::Status putAttr(SchemaAttr *attr);
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsBegin() const;
        absl::flat_hash_map<AttrId,AttrAddress>::const_iterator attrsEnd() const;
        int numAttrs() const;

        bool hasColumn(std::string_view columnId) const;
        ColumnAddress getColumn(int index) const;
        tempo_utils::Status appendColumn(SchemaColumn *column);
        std::vector<ColumnAddress>::const_iterator columnsBegin() const;
        std::vector<ColumnAddress>::const_iterator columnsEnd() const;
        int numColumns() const;

    private:
        std::string m_modelId;
        ModelKeyType m_keyType;
        ModelKeyCollation m_keyCollation;
        ModelAddress m_address;
        SchemaState *m_state;
        absl::flat_hash_map<AttrId,AttrAddress> m_attrs;
        std::vector<ColumnAddress> m_columns;
        absl::flat_hash_map<std::string,tu_int16> m_columnIndex;
    };
}

#endif // GROOVE_MODEL_SCHEMA_MODEL_H
