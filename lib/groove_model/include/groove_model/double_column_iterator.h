#ifndef GROOVE_MODEL_DOUBLE_COLUMN_ITERATOR_H
#define GROOVE_MODEL_DOUBLE_COLUMN_ITERATOR_H

#include <groove_data/double_double_vector.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>
#include <tempo_utils/iterator_template.h>

#include "page_id.h"

namespace groove_model {

    class DoubleDoubleColumnIterator : public Iterator<groove_data::DoubleDoubleDatum> {
    public:
        DoubleDoubleColumnIterator();
        DoubleDoubleColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::DoubleDoubleVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::DoubleDoubleDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::DoubleDoubleVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::DoubleDoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class DoubleInt64ColumnIterator : public Iterator<groove_data::DoubleInt64Datum> {
    public:
        DoubleInt64ColumnIterator();
        DoubleInt64ColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::DoubleInt64Vector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::DoubleInt64Datum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::DoubleInt64Vector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::DoubleInt64Vector> m_vector;
        tu_int64 m_curr;
    };

    class DoubleStringColumnIterator : public Iterator<groove_data::DoubleStringDatum> {
    public:
        DoubleStringColumnIterator();
        DoubleStringColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::DoubleStringVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::DoubleStringDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::DoubleStringVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::DoubleStringVector> m_vector;
        tu_int64 m_curr;
    };
}

#endif // GROOVE_MODEL_DOUBLE_COLUMN_ITERATOR_H