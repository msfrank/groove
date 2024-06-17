#ifndef GROOVE_MODEL_INT64_COLUMN_ITERATOR_H
#define GROOVE_MODEL_INT64_COLUMN_ITERATOR_H

#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>
#include <tempo_utils/iterator_template.h>

#include "page_id.h"

namespace groove_model {

    class Int64DoubleColumnIterator : public Iterator<groove_data::Int64DoubleDatum> {
    public:
        Int64DoubleColumnIterator();
        Int64DoubleColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::Int64DoubleVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::Int64DoubleDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::Int64DoubleVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::Int64DoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class Int64Int64ColumnIterator : public Iterator<groove_data::Int64Int64Datum> {
    public:
        Int64Int64ColumnIterator();
        Int64Int64ColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::Int64Int64Vector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::Int64Int64Datum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::Int64Int64Vector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::Int64Int64Vector> m_vector;
        tu_int64 m_curr;
    };

    class Int64StringColumnIterator : public Iterator<groove_data::Int64StringDatum> {
    public:
        Int64StringColumnIterator();
        Int64StringColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::Int64StringVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::Int64StringDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::Int64StringVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::Int64StringVector> m_vector;
        tu_int64 m_curr;
    };
}

#endif // GROOVE_MODEL_INT64_COLUMN_ITERATOR_H