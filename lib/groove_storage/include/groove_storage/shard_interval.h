#ifndef GROOVE_STORAGE_SHARD_INTERVAL_H
#define GROOVE_STORAGE_SHARD_INTERVAL_H

#include <groove_data/category.h>

#include "storage_types.h"

namespace groove_storage {

    class AbstractShardInterval {
    public:
        virtual ~AbstractShardInterval() = default;

        virtual IntervalType getType() const = 0;
    };

    class CategoryShardInterval : public AbstractShardInterval {
    public:
        CategoryShardInterval(const groove_data::Category &start, const groove_data::Category &end);
        IntervalType getType() const override;
        groove_data::Category getStart() const;
        groove_data::Category getEnd() const;

    private:
        groove_data::Category m_start;
        groove_data::Category m_end;
    };

    class DoubleShardInterval : public AbstractShardInterval {
    public:
        DoubleShardInterval(double start, double end);
        IntervalType getType() const override;
        double getStart() const;
        double getEnd() const;

    private:
        double m_start;
        double m_end;
    };

    class Int64ShardInterval : public AbstractShardInterval {
    public:
        Int64ShardInterval(tu_int64 start, tu_int64 end);
        IntervalType getType() const override;
        tu_int64 getStart() const;
        tu_int64 getEnd() const;

    private:
        tu_int64 m_start;
        tu_int64 m_end;
    };

}

#endif // GROOVE_STORAGE_SHARD_INTERVAL_H