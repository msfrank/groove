#ifndef GROOVE_STORAGE_SHARD_STATE_H
#define GROOVE_STORAGE_SHARD_STATE_H

#include <groove_data/category.h>
#include <tempo_utils/integer_types.h>

#include "groove_shard.h"
#include "shard_interval.h"
#include "storage_result.h"

namespace groove_storage {

    class ShardState {

    public:
        ShardState();

        void setDatasetUrl(const tempo_utils::Url &datasetUrl);
        void setModelId(const std::string &modelId);
        void setTimestamp(tu_uint32 timestamp);
        void setCategoryInterval(const groove_data::Category &start, const groove_data::Category &end);
        void setDoubleInterval(double start, double end);
        void setInt64Interval(tu_int64 start, tu_int64 end);

        tempo_utils::Result<GrooveShard> toShard(bool noIdentifier = false) const;

    private:
        tu_uint32 m_timestamp;
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
        std::unique_ptr<AbstractShardInterval> m_interval;
    };
}

#endif // GROOVE_STORAGE_SHARD_STATE_H