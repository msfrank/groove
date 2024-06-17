
#include <gtest/gtest.h>

#include <groove_model/page_id.h>

TEST(PageId, CreatePageId)
{
    using namespace groove_data;
    using namespace groove_model;

    auto datasetUrl = tempo_utils::Url::fromString("test://dataset");
    auto modelId = std::make_shared<const std::string>("model");
    auto columnId = std::make_shared<const std::string>("column");
    auto pageId = PageId::create<Int64Int64,CollationMode::COLLATION_INDEXED>(
        datasetUrl, modelId, columnId, Option<tu_int64>(0));

    ASSERT_TRUE (pageId.isValid());
    ASSERT_EQ (CollationMode::COLLATION_INDEXED, pageId.getCollation());
    ASSERT_EQ (DataKeyType::KEY_INT64, pageId.getKeyType());
    ASSERT_EQ (DataValueType::VALUE_TYPE_INT64, pageId.getValueType());

    std::string expected("page" "\x1f" "test://dataset" "\x1f" "model" "\x1f" "column" "\x1e");
    ASSERT_EQ (expected, pageId.getPrefix());
}

TEST(PageId, ParsePageIdFromString)
{
    using namespace groove_data;
    using namespace groove_model;

    auto datasetUrl = tempo_utils::Url::fromString("test://dataset");
    auto modelId = std::make_shared<const std::string>("model");
    auto columnId = std::make_shared<const std::string>("column");
    auto srcId = PageId::create<Int64Int64,CollationMode::COLLATION_INDEXED>(
        datasetUrl, modelId, columnId, Option<tu_int64>(0));
    auto srcBytes = srcId.getBytes();
    auto dstId = PageId::fromString(srcBytes);
    auto dstBytes = dstId.getBytes();

    ASSERT_EQ (srcId, dstId);
    ASSERT_EQ (srcBytes, dstBytes);
}