
#include <groove_storage/generated/shard.h>
#include <groove_storage/shard_state.h>
#include <tempo_utils/memory_bytes.h>

groove_storage::ShardState::ShardState()
{
}

void
groove_storage::ShardState::setDatasetUrl(const tempo_utils::Url &datasetUrl)
{
    m_datasetUrl = datasetUrl;
}

void
groove_storage::ShardState::setModelId(const std::string &modelId)
{
    m_modelId = modelId;
}

void
groove_storage::ShardState::setTimestamp(tu_uint32 timestamp)
{
    m_timestamp = timestamp;
}

void
groove_storage::ShardState::setCategoryInterval(const groove_data::Category &start, const groove_data::Category &end)
{
    m_interval = std::make_unique<CategoryShardInterval>(start, end);
}

void
groove_storage::ShardState::setDoubleInterval(double start, double end)
{
    m_interval = std::make_unique<DoubleShardInterval>(start, end);
}

void
groove_storage::ShardState::setInt64Interval(tu_int64 start, tu_int64 end)
{
    m_interval = std::make_unique<Int64ShardInterval>(start, end);
}

inline flatbuffers::Offset<
    flatbuffers::Vector<
        flatbuffers::Offset<flatbuffers::String>>>
write_category(flatbuffers::FlatBufferBuilder &buffer, const groove_data::Category &cat)
{
    std::vector<std::string> cat_vector;
    for (auto iterator = cat.cbegin(); iterator != cat.cend(); iterator++) {
        cat_vector.push_back(**iterator);
    }
    return buffer.CreateVectorOfStrings(cat_vector);
}

tempo_utils::Result<groove_storage::GrooveShard>
groove_storage::ShardState::toShard(bool noIdentifier) const
{
    flatbuffers::FlatBufferBuilder buffer;

    auto fb_datasetUrl = buffer.CreateString(m_datasetUrl.toString());
    auto fb_modelId = buffer.CreateString(m_modelId);

    gss1::Interval fb_intervalType;
    flatbuffers::Offset<void> fb_interval;

    if (m_interval) {
        switch (m_interval->getType()) {
            case IntervalType::Category: {
                auto *cat = static_cast<CategoryShardInterval *>(m_interval.get());
                auto start = write_category(buffer, cat->getStart());
                auto end = write_category(buffer, cat->getEnd());
                auto interval = gss1::CreateCategoryInterval(buffer, start, end);
                fb_intervalType = gss1::Interval::CategoryInterval;
                fb_interval = interval.Union();
                break;
            }
            case IntervalType::Double: {
                auto *dbl = static_cast<DoubleShardInterval *>(m_interval.get());
                auto interval = gss1::CreateDoubleInterval(buffer, dbl->getStart(), dbl->getEnd());
                fb_intervalType = gss1::Interval::DoubleInterval;
                fb_interval = interval.Union();
                break;
            }
            case IntervalType::Int64: {
                auto *i64 = static_cast<Int64ShardInterval *>(m_interval.get());
                auto interval = gss1::CreateInt64Interval(buffer, i64->getStart(), i64->getEnd());
                fb_intervalType = gss1::Interval::Int64Interval;
                fb_interval = interval.Union();
                break;
            }
            default:
                return StorageStatus::forCondition(
                    StorageCondition::kStorageInvariant, "invalid interval type");
        }
    }

    // build shard from buffer
    gss1::ShardBuilder shardBuilder(buffer);

    shardBuilder.add_abi(gss1::ShardVersion::Version1);
    shardBuilder.add_timestamp(m_timestamp);
    shardBuilder.add_dataset_uri(fb_datasetUrl);
    shardBuilder.add_model_id(fb_modelId);

    if (m_interval) {
        shardBuilder.add_interval_type(fb_intervalType);
        shardBuilder.add_interval(fb_interval.Union());
    }

    // serialize shard and mark the buffer as finished
    auto shard = shardBuilder.Finish();
    if (noIdentifier) {
        buffer.Finish(shard);
    } else {
        buffer.Finish(shard, gss1::ShardIdentifier());
    }

    // copy the flatbuffer into our own byte array and instantiate shard
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return GrooveShard(bytes);
}