
#include <groove_model/double_column_iterator.h>
#include <tempo_utils/log_stream.h>

groove_model::DoubleDoubleColumnIterator::DoubleDoubleColumnIterator()
    : m_curr(-1)
{
}

groove_model::DoubleDoubleColumnIterator::DoubleDoubleColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::DoubleDoubleVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::DoubleDoubleColumnIterator::getNext(groove_data::DoubleDoubleDatum &datum)
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
groove_model::DoubleDoubleColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::DoubleDoubleColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::DoubleInt64ColumnIterator::DoubleInt64ColumnIterator()
    : m_curr(-1)
{
}

groove_model::DoubleInt64ColumnIterator::DoubleInt64ColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::DoubleInt64Vector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::DoubleInt64ColumnIterator::getNext(groove_data::DoubleInt64Datum &datum)
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
groove_model::DoubleInt64ColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::DoubleInt64ColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}

groove_model::DoubleStringColumnIterator::DoubleStringColumnIterator()
    : m_curr(-1)
{
}

groove_model::DoubleStringColumnIterator::DoubleStringColumnIterator(
    const std::forward_list<std::shared_ptr<groove_data::DoubleStringVector>> &vectors,
    const std::vector<PageId> &pageIds)
    : m_vectors(vectors),
      m_pageIds(pageIds),
      m_curr(0)
{
}

bool
groove_model::DoubleStringColumnIterator::getNext(groove_data::DoubleStringDatum &datum)
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
groove_model::DoubleStringColumnIterator::pageIdsBegin() const
{
    return m_pageIds.cbegin();
}

std::vector<groove_model::PageId>::const_iterator
groove_model::DoubleStringColumnIterator::pageIdsEnd() const
{
    return m_pageIds.cend();
}
