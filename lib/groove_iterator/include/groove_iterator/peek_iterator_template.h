#ifndef GROOVE_ITERATOR_PEEK_ITERATOR_TEMPLATE_H
#define GROOVE_ITERATOR_PEEK_ITERATOR_TEMPLATE_H

#include "base_iterator.h"

namespace groove_iterator {

    template<class InputIteratorType, class InputType>
    class PeekIterator : public BaseIterator<InputType> {
    public:
        PeekIterator(InputIteratorType input);

        bool peekNext(InputType &value);
        bool getNext(InputType &value);

    private:
        InputIteratorType m_input;
        Option<InputType> m_peek;
    };

    template<class InputIteratorType, class InputType>
    PeekIterator<InputIteratorType,InputType>::PeekIterator(InputIteratorType input) : m_input(input)
    {
    }

    template<class InputIteratorType, class InputType>
    bool
    PeekIterator<InputIteratorType,InputType>::peekNext(InputType &value)
    {
        if (!m_peek.isEmpty()) {
            value = m_peek.getValue();
            return true;
        } else {
            InputType peek;
            bool valid = getNext(peek);
            if (!valid)
                return false;
            m_peek = Option<InputType>(peek);
            value = peek;
            return true;
        }
    }

    template<class InputIteratorType, class InputType>
    bool
    PeekIterator<InputIteratorType,InputType>::getNext(InputType &value)
    {
        if (!m_peek.isEmpty()) {
            value = m_peek.getValue();
            m_peek = Option<InputType>();
            return true;
        } else {
            return getNext(value);
        }
    }
}

#endif // GROOVE_ITERATOR_PEEK_ITERATOR_TEMPLATE_H