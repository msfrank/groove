#ifndef GROOVE_MATH_REDUCER_TRAITS_H
#define GROOVE_MATH_REDUCER_TRAITS_H

#include "average_function.h"
#include "math_types.h"
#include "math_result.h"
#include "maximum_function.h"
#include "minimum_function.h"
#include "samplecount_function.h"
#include "sum_function.h"

namespace groove_math {

    template <>
    struct ReducerTraits<Average> {
        using DefType = Average;
        using FunctionType = AverageFunction;
        using InputType = double;
        using OutputType = double;
    };

    template <>
    struct ReducerTraits<Maximum> {
        using DefType = Maximum;
        using FunctionType = MaximumFunction;
        using InputType = double;
        using OutputType = double;
    };

    template <>
    struct ReducerTraits<Minimum> {
        using DefType = Minimum;
        using FunctionType = MinimumFunction;
        using InputType = double;
        using OutputType = double;
    };

    template <>
    struct ReducerTraits<SampleCount> {
        using DefType = SampleCount;
        using FunctionType = SampleCountFunction;
        using InputType = double;
        using OutputType = double;
    };

    template <>
    struct ReducerTraits<Sum> {
        using DefType = Sum;
        using FunctionType = SumFunction;
        using InputType = double;
        using OutputType = double;
    };
}

#endif // GROOVE_MATH_REDUCER_TRAITS_H