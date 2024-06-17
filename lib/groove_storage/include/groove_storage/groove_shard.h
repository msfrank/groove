#ifndef GROOVE_STORAGE_GROOVE_SHARD_H
#define GROOVE_STORAGE_GROOVE_SHARD_H

#include <span>

#include <absl/strings/string_view.h>

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/integer_types.h>
#include <tempo_utils/url.h>

#include "storage_types.h"

namespace groove_storage {

    class GrooveShard {

    public:
        GrooveShard();
        GrooveShard(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes);
        GrooveShard(std::span<const tu_uint8> unownedBytes);
        GrooveShard(const GrooveShard &other);

        bool isValid() const;

        ShardVersion getABI() const;

        tu_uint32 getTimestamp() const;
        tempo_utils::Url getDatasetUrl() const;
        std::string getModelId() const;

        IntervalType getIntervalType() const;

        const groove_data::Category getCategoryIntervalStart() const;
        const groove_data::Category getCategoryIntervalEnd() const;

        double getDoubleIntervalStart() const;
        double getDoubleIntervalEnd() const;

        tu_int64 getInt64IntervalStart() const;
        tu_int64 getInt64IntervalEnd() const;

        std::shared_ptr<const internal::ShardReader> getReader() const;
        std::span<const tu_uint8> bytesView() const;

        static bool verify(std::span<const tu_uint8> bytes);

    private:
        std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
        std::shared_ptr<const internal::ShardReader> m_reader;
    };
}

#endif // GROOVE_STORAGE_GROOVE_SHARD_H