#ifndef GROOVE_ITERATOR_RANGE_ITERATOR_TEMPLATE_H
#define GROOVE_ITERATOR_RANGE_ITERATOR_TEMPLATE_H

#include "base_iterator.h"

namespace groove_iterator {

    template<typename ContainerType>
    class RangeIterator : public BaseIterator<typename ContainerType::value_type> {

        using ValueType = typename ContainerType::value_type;
        using ConstIteratorType = typename ContainerType::const_iterator;

    public:
        RangeIterator(
            std::shared_ptr<const ContainerType> container,
            ConstIteratorType itBegin,
            ConstIteratorType itEnd);

        bool getNext(ValueType &value) override;

    private:
        std::shared_ptr<const ContainerType> m_container;
        ConstIteratorType m_curr;
        ConstIteratorType m_end;
    };

    template<typename ContainerType>
    RangeIterator<ContainerType>::RangeIterator(
        std::shared_ptr<const ContainerType> container,
        ConstIteratorType begin,
        ConstIteratorType end)
        : m_container(container),
          m_curr(begin),
          m_end(end)
    {
    }

    template<typename ContainerType>
    bool
    RangeIterator<ContainerType>::getNext(ValueType &value)
    {
        if (m_curr == m_end)
            return false;
        value = *m_curr++;
        return true;
    }
}

#endif // GROOVE_ITERATOR_RANGE_ITERATOR_TEMPLATE_H