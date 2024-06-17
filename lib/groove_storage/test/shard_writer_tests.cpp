#include <gtest/gtest.h>

#include <groove_storage/shard_state.h>

TEST(ShardWriterTest, TestWriteShard)
{
    auto datasetUrl = tempo_utils::Url::fromString("/test");
    std::string modelId("model");
    tu_uint32 timestamp = ToUnixSeconds(absl::Now());

    groove_storage::ShardState writer;
    writer.setDatasetUrl(datasetUrl);
    writer.setModelId(modelId);
    writer.setTimestamp(timestamp);

    auto toShardResult = writer.toShard();
    ASSERT_TRUE (toShardResult.isResult());
    auto shard = toShardResult.getResult();
    ASSERT_EQ (datasetUrl, shard.getDatasetUrl());
    ASSERT_EQ (modelId, shard.getModelId());
    ASSERT_EQ (timestamp, shard.getTimestamp());
    ASSERT_EQ (groove_storage::IntervalType::Invalid, shard.getIntervalType());
}

TEST(ShardWriterTest, TestWriteShardWithCategoryInterval)
{
    auto datasetUrl = tempo_utils::Url::fromString("/test");
    std::string modelId("model");
    tu_uint32 timestamp = ToUnixSeconds(absl::Now());
    auto categoryStart = groove_data::Category({"a"});
    auto categoryEnd = groove_data::Category({"z"});

    groove_storage::ShardState writer;
    writer.setDatasetUrl(datasetUrl);
    writer.setModelId(modelId);
    writer.setTimestamp(timestamp);
    writer.setCategoryInterval(categoryStart, categoryEnd);

    auto toShardResult = writer.toShard();
    ASSERT_TRUE (toShardResult.isResult());
    auto shard = toShardResult.getResult();
    ASSERT_EQ (datasetUrl, shard.getDatasetUrl());
    ASSERT_EQ (modelId, shard.getModelId());
    ASSERT_EQ (timestamp, shard.getTimestamp());
    ASSERT_EQ (groove_storage::IntervalType::Category, shard.getIntervalType());
    auto start = shard.getCategoryIntervalStart();
    auto end = shard.getCategoryIntervalEnd();
    ASSERT_EQ (categoryStart, shard.getCategoryIntervalStart());
    ASSERT_EQ (categoryEnd, shard.getCategoryIntervalEnd());
}

TEST(ShardWriterTest, TestWriteShardWithDoubleInterval)
{
    auto datasetUrl = tempo_utils::Url::fromString("/test");
    std::string modelId("model");
    tu_uint32 timestamp = ToUnixSeconds(absl::Now());

    groove_storage::ShardState writer;
    writer.setDatasetUrl(datasetUrl);
    writer.setModelId(modelId);
    writer.setTimestamp(timestamp);
    writer.setDoubleInterval(0.1, 0.5);

    auto toShardResult = writer.toShard();
    ASSERT_TRUE (toShardResult.isResult());
    auto shard = toShardResult.getResult();
    ASSERT_EQ (datasetUrl, shard.getDatasetUrl());
    ASSERT_EQ (modelId, shard.getModelId());
    ASSERT_EQ (timestamp, shard.getTimestamp());
    ASSERT_EQ (groove_storage::IntervalType::Double, shard.getIntervalType());
    ASSERT_DOUBLE_EQ(0.1, shard.getDoubleIntervalStart());
    ASSERT_DOUBLE_EQ(0.5, shard.getDoubleIntervalEnd());
}

TEST(ShardWriterTest, TestWriteShardWithInt64Interval)
{
    auto datasetUrl = tempo_utils::Url::fromString("/test");
    std::string modelId("model");
    tu_uint32 timestamp = ToUnixSeconds(absl::Now());

    groove_storage::ShardState writer;
    writer.setDatasetUrl(datasetUrl);
    writer.setModelId(modelId);
    writer.setTimestamp(timestamp);
    writer.setInt64Interval(0, 10);

    auto toShardResult = writer.toShard();
    ASSERT_TRUE (toShardResult.isResult());
    auto shard = toShardResult.getResult();
    ASSERT_EQ (datasetUrl, shard.getDatasetUrl());
    ASSERT_EQ (modelId, shard.getModelId());
    ASSERT_EQ (timestamp, shard.getTimestamp());
    ASSERT_EQ (groove_storage::IntervalType::Int64, shard.getIntervalType());
    ASSERT_EQ (0, shard.getInt64IntervalStart());
    ASSERT_EQ (10, shard.getInt64IntervalEnd());
}
