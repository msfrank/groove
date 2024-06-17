#ifndef GROOVE_MODEL_GROOVE_SCHEMA_H
#define GROOVE_MODEL_GROOVE_SCHEMA_H

#include <span>

#include <groove_model/model_types.h>
#include <groove_model/schema_walker.h>
#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/integer_types.h>

namespace groove_model {

    class GrooveSchema {

    public:
        GrooveSchema();
        GrooveSchema(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        GrooveSchema(std::span<const tu_uint8> unownedBytes);
        GrooveSchema(const GrooveSchema &other);

        bool isValid() const;

        SchemaVersion getABI() const;

        SchemaWalker getSchema() const;

        std::shared_ptr<const internal::SchemaReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        static bool verify(std::span<const tu_uint8> bytes, bool noIdentifier = false);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::SchemaReader> m_reader;
    };
}

#endif // GROOVE_MODEL_GROOVE_SCHEMA_H