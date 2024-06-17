#ifndef GROOVE_ITERATOR_FILTER_ITERATOR_TEMPLATE_H
#define GROOVE_ITERATOR_FILTER_ITERATOR_TEMPLATE_H

#include <tempo_utils/log_stream.h>

#include "base_iterator.h"

namespace groove_iterator {

    template<class InputIteratorType, class InputType>
    class FilterIterator : public BaseIterator<InputType> {

    public:
        typedef bool (*FilterFunction)(const InputType &);

        FilterIterator(InputIteratorType input, FilterFunction func);

        bool getNext(InputType &value) override;

    private:
        InputIteratorType m_input;
        FilterFunction m_func;
    };

    template<class InputIteratorType, class InputType>
    FilterIterator<InputIteratorType, InputType>::FilterIterator(
        InputIteratorType input,
        FilterFunction func)
        : m_input(input),
          m_func(func)
    {
        TU_ASSERT (m_func != nullptr);
    }

    template<class InputIteratorType, class InputType>
    bool
    FilterIterator<InputIteratorType, InputType>::getNext(InputType &value)
    {
        InputType input;
        while (m_input.getNext(input)) {
            if (m_func(input)) {
                value = input;
                return true;
            }
        }
        return false;
    }
}

#endif // GROOVE_ITERATOR_FILTER_ITERATOR_TEMPLATE_H