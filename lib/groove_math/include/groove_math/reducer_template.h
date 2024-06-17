#ifndef GROOVE_MATH_REDUCER_TEMPLATE_H
#define GROOVE_MATH_REDUCER_TEMPLATE_H

#include <tempo_utils/iterator_template.h>

#include "base_reducer.h"
#include "math_result.h"
#include "reducer_traits.h"

namespace groove_math {

    template<typename DefType,
        typename FunctionType = typename ReducerTraits<DefType>::FunctionType,
        typename InputType = typename ReducerTraits<DefType>::InputType,
        typename OutputType = typename ReducerTraits<DefType>::OutputType>
    class Reducer : public BaseReducer {

    public:
        typedef MathStatus (*ReduceFunction)(OutputType &, const InputType &);

        Reducer(const OutputType &identity)
            : m_identity(identity),
              m_reducer(FunctionType())
        {
        }

        Reducer(const OutputType &identity, const FunctionType &reducer)
            : m_identity(identity),
              m_reducer(reducer)
        {
        }

    private:
        const OutputType m_identity;
        FunctionType m_reducer;

    public:

        tempo_utils::Result<OutputType>
        reduce(std::shared_ptr<Iterator<InputType>> input)
        {
            OutputType output = m_identity;

            InputType next;
            while (input->getNext(next)) {
                MathStatus status = m_reducer.apply(output, next);
                if (status.notOk())
                    return status;
            }
            return output;
        }
    };
}

#endif // GROOVE_MATH_REDUCER_TEMPLATE_H