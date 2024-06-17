
#include <groove_iterator/base_iterator.h>

groove_iterator::ErrorTrackingIterator::ErrorTrackingIterator()
    : m_error(false)
{
}

bool
groove_iterator::ErrorTrackingIterator::getError() const
{
    return m_error;
}

void
groove_iterator::ErrorTrackingIterator::setError()
{
    m_error = true;
}