#ifndef GROOVE_STORAGE_INTERNAL_SHARD_READER_H
#define GROOVE_STORAGE_INTERNAL_SHARD_READER_H

#include <span>

#include <groove_storage/generated/shard.h>
#include <tempo_utils/integer_types.h>

namespace groove_storage::internal {

    class ShardReader {

    public:
        ShardReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        gss1::ShardVersion getABI() const;
        tu_uint32 getTimestamp() const;
        std::string getDatasetUri() const;
        std::string_view datasetUriView() const;
        std::string getModelId() const;
        std::string_view modelIdView() const;

        gss1::Interval getIntervalType() const;
        const gss1::CategoryInterval *getCategoryInterval() const;
        const gss1::DoubleInterval *getDoubleInterval() const;
        const gss1::Int64Interval *getInt64Interval() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const gss1::Shard *m_shard;
    };
}

#endif // GROOVE_STORAGE_INTERNAL_SHARD_READER_H