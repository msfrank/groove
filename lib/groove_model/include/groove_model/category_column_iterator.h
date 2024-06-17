#ifndef GROOVE_MODEL_CATEGORY_COLUMN_ITERATOR_H
#define GROOVE_MODEL_CATEGORY_COLUMN_ITERATOR_H

#include <groove_data/category_double_vector.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <tempo_utils/iterator_template.h>

#include "page_id.h"

namespace groove_model {

    class CategoryDoubleColumnIterator : public Iterator<groove_data::CategoryDoubleDatum> {
    public:
        CategoryDoubleColumnIterator();
        CategoryDoubleColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::CategoryDoubleVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::CategoryDoubleDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::CategoryDoubleVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::CategoryDoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class CategoryInt64ColumnIterator : public Iterator<groove_data::CategoryInt64Datum> {
    public:
        CategoryInt64ColumnIterator();
        CategoryInt64ColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::CategoryInt64Vector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::CategoryInt64Datum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::CategoryInt64Vector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::CategoryInt64Vector> m_vector;
        tu_int64 m_curr;
    };

    class CategoryStringColumnIterator : public Iterator<groove_data::CategoryStringDatum> {
    public:
        CategoryStringColumnIterator();
        CategoryStringColumnIterator(
            const std::forward_list<std::shared_ptr<groove_data::CategoryStringVector>> &vectors,
            const std::vector<PageId> &pageIds);

        bool getNext(groove_data::CategoryStringDatum &datum) override;
        std::vector<PageId>::const_iterator pageIdsBegin() const;
        std::vector<PageId>::const_iterator pageIdsEnd() const;

    private:
        std::forward_list<std::shared_ptr<groove_data::CategoryStringVector>> m_vectors;
        std::vector<PageId> m_pageIds;
        std::shared_ptr<groove_data::CategoryStringVector> m_vector;
        tu_int64 m_curr;
    };
}

#endif // GROOVE_MODEL_CATEGORY_COLUMN_ITERATOR_H