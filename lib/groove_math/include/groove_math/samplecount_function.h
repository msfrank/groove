#ifndef GROOVE_MATH_SAMPLECOUNT_FUNCTION_H
#define GROOVE_MATH_SAMPLECOUNT_FUNCTION_H

#include "math_result.h"

namespace groove_math {

    class SampleCountFunction {

    public:
        SampleCountFunction();

        MathStatus apply(double &output, const double &input);

    private:
        double m_count;
    };
}

#endif // GROOVE_MATH_SAMPLECOUNT_FUNCTION_H