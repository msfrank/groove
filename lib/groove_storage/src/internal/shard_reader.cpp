
#include <flatbuffers/idl.h>

#include <groove_storage/generated/shard_schema.h>
#include <groove_storage/internal/shard_reader.h>
#include <tempo_utils/log_stream.h>

groove_storage::internal::ShardReader::ShardReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_shard = gss1::GetShard(m_bytes.data());
}

bool
groove_storage::internal::ShardReader::isValid() const
{
    return m_shard != nullptr;
}

gss1::ShardVersion
groove_storage::internal::ShardReader::getABI() const
{
    if (m_shard == nullptr)
        return gss1::ShardVersion::Unknown;
    return m_shard->abi();
}

tu_uint32
groove_storage::internal::ShardReader::getTimestamp() const
{
    if (m_shard == nullptr)
        return 0;
    return m_shard->timestamp();
}

std::string
groove_storage::internal::ShardReader::getDatasetUri() const
{
    if (m_shard == nullptr)
        return {};
    auto *uri = m_shard->dataset_uri();
    if (uri == nullptr)
        return {};
    return uri->str();
}

std::string_view
groove_storage::internal::ShardReader::datasetUriView() const
{
    if (m_shard == nullptr)
        return 0;
    auto *uri = m_shard->dataset_uri();
    if (uri == nullptr)
        return {};
    return uri->string_view();
}

std::string
groove_storage::internal::ShardReader::getModelId() const
{
    if (m_shard == nullptr)
        return 0;
    auto *id = m_shard->model_id();
    if (id == nullptr)
        return {};
    return id->str();
}

std::string_view
groove_storage::internal::ShardReader::modelIdView() const
{
    if (m_shard == nullptr)
        return 0;
    auto *id = m_shard->model_id();
    if (id == nullptr)
        return {};
    return id->string_view();
}

gss1::Interval
groove_storage::internal::ShardReader::getIntervalType() const
{
    if (m_shard == nullptr)
        return gss1::Interval::NONE;
    return m_shard->interval_type();
}

const gss1::CategoryInterval *
groove_storage::internal::ShardReader::getCategoryInterval() const
{
    if (m_shard == nullptr)
        return nullptr;
    if (m_shard->interval_type() != gss1::Interval::CategoryInterval)
        return nullptr;
    return m_shard->interval_as_CategoryInterval();
}

const gss1::DoubleInterval *
groove_storage::internal::ShardReader::getDoubleInterval() const
{
    if (m_shard == nullptr)
        return nullptr;
    if (m_shard->interval_type() != gss1::Interval::DoubleInterval)
        return nullptr;
    return m_shard->interval_as_DoubleInterval();
}

const gss1::Int64Interval *
groove_storage::internal::ShardReader::getInt64Interval() const
{
    if (m_shard == nullptr)
        return nullptr;
    if (m_shard->interval_type() != gss1::Interval::Int64Interval)
        return nullptr;
    return m_shard->interval_as_Int64Interval();
}

std::span<const tu_uint8>
groove_storage::internal::ShardReader::bytesView() const
{
    return m_bytes;
}

std::string
groove_storage::internal::ShardReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) groove_storage::shard::schema::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
