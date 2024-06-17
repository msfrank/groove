
#include <groove_model/category_column_iterator.h>
#include <tempo_utils/log_stream.h>

groove_model::CategoryDoubleColumnIterator::CategoryDoubleColumnIterator()
    : m_curr(-1)
{
}

groove_model::CategoryDoubleColumnIterator::CategoryDoubleColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::CategoryDoubleVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::CategoryDoubleColumnIterator::getNext(groove_data::CategoryDoubleDatum &datum)
{
    if (m_vector == nullptr || m_vector->getSize() <= m_curr) {
        if (m_vectors.empty())
            return false;
        m_vector = m_vectors.front();
        m_vectors.pop_front();
        m_curr = 0;
    }
    datum = m_vector->getDatum(m_curr++);
    return true;
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryDoubleColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryDoubleColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::CategoryInt64ColumnIterator::CategoryInt64ColumnIterator()
    : m_curr(-1)
{
}

groove_model::CategoryInt64ColumnIterator::CategoryInt64ColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::CategoryInt64Vector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::CategoryInt64ColumnIterator::getNext(groove_data::CategoryInt64Datum &datum)
{
    if (m_vector == nullptr || m_vector->getSize() <= m_curr) {
        if (m_vectors.empty())
            return false;
        m_vector = m_vectors.front();
        m_vectors.pop_front();
        m_curr = 0;
    }
    datum = m_vector->getDatum(m_curr++);
    return true;
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryInt64ColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryInt64ColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::CategoryStringColumnIterator::CategoryStringColumnIterator()
    : m_curr(-1)
{
}

groove_model::CategoryStringColumnIterator::CategoryStringColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::CategoryStringVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::CategoryStringColumnIterator::getNext(groove_data::CategoryStringDatum &datum)
{
    if (m_vector == nullptr || m_vector->getSize() <= m_curr) {
        if (m_vectors.empty())
            return false;
        m_vector = m_vectors.front();
        m_vectors.pop_front();
        m_curr = 0;
    }
    datum = m_vector->getDatum(m_curr++);
    return true;
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryStringColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::CategoryStringColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}
