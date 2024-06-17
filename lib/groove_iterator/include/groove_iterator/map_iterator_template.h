#ifndef GROOVE_ITERATOR_MAP_ITERATOR_TEMPLATE_H
#define GROOVE_ITERATOR_MAP_ITERATOR_TEMPLATE_H

#include <tempo_utils/log_stream.h>

#include "base_iterator.h"

namespace groove_iterator {

    template<class InputIteratorType, class InputType, class OutputType>
    class MapIterator : public BaseIterator<OutputType> {

    public:
        typedef OutputType (*MapFunction)(const InputType &);

        MapIterator(InputIteratorType input, MapFunction func);

        bool getNext(OutputType &value) override;

    private:
        InputIteratorType m_input;
        MapFunction m_func;
    };

    template<class InputIteratorType, class InputType, class OutputType>
    MapIterator<InputIteratorType, InputType, OutputType>::MapIterator(
        InputIteratorType input,
        MapFunction func)
        : m_input(input),
          m_func(func)
    {
        TU_ASSERT (m_func != nullptr);
    }

    template<class InputIteratorType, class InputType, class OutputType>
    bool
    MapIterator<InputIteratorType, InputType, OutputType>::getNext(OutputType &value)
    {
        InputType input;
        if (!m_input.getNext(input))
            return false;
        value = m_func(input);
        return true;
    }
}

#endif // GROOVE_ITERATOR_MAP_ITERATOR_TEMPLATE_H