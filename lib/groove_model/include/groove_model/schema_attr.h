#ifndef GROOVE_MODEL_SCHEMA_ATTR_H
#define GROOVE_MODEL_SCHEMA_ATTR_H

#include <tempo_utils/attr.h>

#include "model_types.h"
#include "schema_state.h"

namespace groove_model {

    class SchemaAttr {

    public:
        SchemaAttr(
            AttrId id,
            tempo_utils::AttrValue value,
            AttrAddress address,
            SchemaState *state);

        AttrId getAttrId() const;
        tempo_utils::AttrValue getAttrValue() const;
        AttrAddress getAddress() const;

    private:
        AttrId m_id;
        tempo_utils::AttrValue m_value;
        AttrAddress m_address;
        SchemaState *m_state;
    };
}

#endif // GROOVE_MODEL_SCHEMA_ATTR_H
