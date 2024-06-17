
#include <groove_storage/shard_interval.h>

groove_storage::CategoryShardInterval::CategoryShardInterval(
    const groove_data::Category &start,
    const groove_data::Category &end)
    : m_start(start),
      m_end(end)
{
}

groove_storage::IntervalType
groove_storage::CategoryShardInterval::getType() const
{
    return IntervalType::Category;
}

groove_data::Category
groove_storage::CategoryShardInterval::getStart() const
{
    return m_start;
}

groove_data::Category
groove_storage::CategoryShardInterval::getEnd() const
{
    return m_end;
}

groove_storage::DoubleShardInterval::DoubleShardInterval(double start, double end)
    : m_start(start),
      m_end(end)
{
}

groove_storage::IntervalType
groove_storage::DoubleShardInterval::getType() const
{
    return IntervalType::Double;
}

double
groove_storage::DoubleShardInterval::getStart() const
{
    return m_start;
}

double
groove_storage::DoubleShardInterval::getEnd() const
{
    return m_end;
}

groove_storage::Int64ShardInterval::Int64ShardInterval(tu_int64 start, tu_int64 end)
    : m_start(start),
      m_end(end)
{
}

groove_storage::IntervalType
groove_storage::Int64ShardInterval::getType() const
{
    return IntervalType::Int64;
}

tu_int64
groove_storage::Int64ShardInterval::getStart() const
{
    return m_start;
}

tu_int64
groove_storage::Int64ShardInterval::getEnd() const
{
    return m_end;
}