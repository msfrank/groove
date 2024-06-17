#ifndef GROOVE_IO_GROOVE_INDEX_H
#define GROOVE_IO_GROOVE_INDEX_H

#include <span>

#include <absl/strings/string_view.h>

#include <groove_io/generated/index.h>
#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/integer_types.h>

#include "index_walker.h"
#include "io_types.h"

namespace groove_io {

    class GrooveIndex {

    public:
        GrooveIndex();
        GrooveIndex(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        GrooveIndex(std::span<const tu_uint8> unownedBytes);
        GrooveIndex(const GrooveIndex &other);

        bool isValid() const;

        IndexVersion getABI() const;

        IndexWalker getIndex() const;

        std::shared_ptr<const internal::IndexReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        static bool verify(std::span<const tu_uint8> data, bool noIdentifier = false);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::IndexReader> m_reader;
    };
}

#endif // GROOVE_IO_GROOVE_INDEX_H