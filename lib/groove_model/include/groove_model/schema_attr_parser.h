#ifndef GROOVE_MODEL_SCHEMA_ATTR_PARSER_H
#define GROOVE_MODEL_SCHEMA_ATTR_PARSER_H

#include <tempo_utils/attr.h>

#include "model_types.h"

namespace groove_model {

    class SchemaAttrParser : public tempo_utils::AbstractAttrParser {

    public:
        explicit SchemaAttrParser(std::shared_ptr<const internal::SchemaReader> reader);
        tempo_utils::Status getNil(tu_uint32 index, std::nullptr_t &nil) override;
        tempo_utils::Status getBool(tu_uint32 index, bool &b) override;
        tempo_utils::Status getInt64(tu_uint32 index, tu_int64 &i64) override;
        tempo_utils::Status getFloat64(tu_uint32 index, double &dbl) override;
        tempo_utils::Status getUInt64(tu_uint32 index, tu_uint64 &u64) override;
        tempo_utils::Status getUInt32(tu_uint32 index, tu_uint32 &u32) override;
        tempo_utils::Status getUInt16(tu_uint32 index, tu_uint16 &u16) override;
        tempo_utils::Status getUInt8(tu_uint32 index, tu_uint8 &u8) override;
        tempo_utils::Status getString(tu_uint32 index, std::string &str) override;

    private:
        std::shared_ptr<const internal::SchemaReader> m_reader;
    };
}

#endif // GROOVE_MODEL_SCHEMA_ATTR_PARSER_H
