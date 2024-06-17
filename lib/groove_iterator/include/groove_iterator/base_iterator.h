#ifndef GROOVE_ITERATOR_BASE_ITERATOR_H
#define GROOVE_ITERATOR_BASE_ITERATOR_H

#include <tempo_utils/iterator_template.h>

namespace groove_iterator {

    class ErrorTrackingIterator {
    public:
        ErrorTrackingIterator();
        virtual ~ErrorTrackingIterator() = default;

        bool getError() const;
        void setError();

    private:
        bool m_error;
    };

    template <typename T>
    class BaseIterator : public Iterator<T>, public ErrorTrackingIterator {
    public:
        virtual ~BaseIterator() = default;
    };
}

#endif // GROOVE_ITERATOR_BASE_ITERATOR_H