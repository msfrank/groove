
#include <flatbuffers/flatbuffers.h>

#include <groove_storage/groove_shard.h>
#include <groove_storage/internal/shard_reader.h>

groove_storage::GrooveShard::GrooveShard()
{
}

groove_storage::GrooveShard::GrooveShard(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::ShardReader>(bytes);
}

groove_storage::GrooveShard::GrooveShard(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::ShardReader>(unownedBytes);
}

groove_storage::GrooveShard::GrooveShard(const groove_storage::GrooveShard &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
groove_storage::GrooveShard::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_storage::ShardVersion
groove_storage::GrooveShard::getABI() const
{
    if (!isValid())
        return ShardVersion::Unknown;
    switch (m_reader->getABI()) {
        case gss1::ShardVersion::Version1:
            return ShardVersion::Version1;
        case gss1::ShardVersion::Unknown:
        default:
            return ShardVersion::Unknown;
    }
}

tu_uint32
groove_storage::GrooveShard::getTimestamp() const
{
    if (!isValid())
        return 0;
    return m_reader->getTimestamp();
}

tempo_utils::Url
groove_storage::GrooveShard::getDatasetUrl() const
{
    if (!isValid())
        return {};
    return tempo_utils::Url::fromString(m_reader->datasetUriView());
}

std::string
groove_storage::GrooveShard::getModelId() const
{
    if (!isValid())
        return {};
    return m_reader->getModelId();
}

groove_storage::IntervalType
groove_storage::GrooveShard::getIntervalType() const
{
    if (!isValid())
        return IntervalType::Invalid;
    switch (m_reader->getIntervalType()) {
        case gss1::Interval::CategoryInterval:
            return IntervalType::Category;
        case gss1::Interval::DoubleInterval:
            return IntervalType::Double;
        case gss1::Interval::Int64Interval:
            return IntervalType::Int64;
        default:
            return IntervalType::Invalid;
    }
}

inline groove_data::Category
read_category(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *parts)
{
    if (parts == nullptr)
        return {};
    std::vector<std::shared_ptr<const std::string>> p;
    for (const auto &part : *parts) {
        p.push_back(std::make_shared<const std::string>(part->string_view()));
    }
    return groove_data::Category(p);
}

const groove_data::Category
groove_storage::GrooveShard::getCategoryIntervalStart() const
{
    if (getIntervalType() != IntervalType::Category)
        return {};
    auto *cat = m_reader->getCategoryInterval();
    if (cat == nullptr)
        return {};
    return read_category(cat->start());
}

const groove_data::Category
groove_storage::GrooveShard::getCategoryIntervalEnd() const
{
    if (getIntervalType() != IntervalType::Category)
        return {};
    auto *cat = m_reader->getCategoryInterval();
    if (cat == nullptr)
        return {};
    return read_category(cat->end());
}

double
groove_storage::GrooveShard::getDoubleIntervalStart() const
{
    if (getIntervalType() != IntervalType::Double)
        return 0.0;
    auto *dbl = m_reader->getDoubleInterval();
    if (dbl == nullptr)
        return 0.0;
    return dbl->start();
}

double
groove_storage::GrooveShard::getDoubleIntervalEnd() const
{
    if (getIntervalType() != IntervalType::Double)
        return 0.0;
    auto *dbl = m_reader->getDoubleInterval();
    if (dbl == nullptr)
        return 0.0;
    return dbl->end();
}

tu_int64
groove_storage::GrooveShard::getInt64IntervalStart() const
{
    if (getIntervalType() != IntervalType::Int64)
        return 0;
    auto *i64 = m_reader->getInt64Interval();
    if (i64 == nullptr)
        return 0;
    return i64->start();
}

tu_int64
groove_storage::GrooveShard::getInt64IntervalEnd() const
{
    if (getIntervalType() != IntervalType::Int64)
        return 0;
    auto *i64 = m_reader->getInt64Interval();
    if (i64 == nullptr)
        return 0;
    return i64->end();
}

std::span<const tu_uint8>
groove_storage::GrooveShard::bytesView() const
{
    if (!isValid())
        return {};
    return m_reader->bytesView();
}

bool
groove_storage::GrooveShard::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return gss1::VerifyShardBuffer(verifier);
}