
#include <groove_model/int64_column_iterator.h>
#include <tempo_utils/log_stream.h>

groove_model::Int64DoubleColumnIterator::Int64DoubleColumnIterator()
    : m_curr(-1)
{
}

groove_model::Int64DoubleColumnIterator::Int64DoubleColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::Int64DoubleVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::Int64DoubleColumnIterator::getNext(groove_data::Int64DoubleDatum &datum)
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
groove_model::Int64DoubleColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::Int64DoubleColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::Int64Int64ColumnIterator::Int64Int64ColumnIterator()
    : m_curr(-1)
{
}

groove_model::Int64Int64ColumnIterator::Int64Int64ColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::Int64Int64Vector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::Int64Int64ColumnIterator::getNext(groove_data::Int64Int64Datum &datum)
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
groove_model::Int64Int64ColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::Int64Int64ColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::Int64StringColumnIterator::Int64StringColumnIterator()
    : m_curr(-1)
{
}

groove_model::Int64StringColumnIterator::Int64StringColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::Int64StringVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::Int64StringColumnIterator::getNext(groove_data::Int64StringDatum &datum)
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
groove_model::Int64StringColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::Int64StringColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}
