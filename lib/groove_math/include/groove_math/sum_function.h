#ifndef GROOVE_MATH_SUM_FUNCTION_H
#define GROOVE_MATH_SUM_FUNCTION_H

#include "math_result.h"

namespace groove_math {

    class SumFunction {

    public:
        SumFunction();

        MathStatus apply(double &output, const double &input);

    private:
        double m_sum;
    };
}

#endif // GROOVE_MATH_SUM_FUNCTION_H