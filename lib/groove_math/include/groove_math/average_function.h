#ifndef GROOVE_MATH_AVERAGE_FUNCTION_H
#define GROOVE_MATH_AVERAGE_FUNCTION_H

#include "math_result.h"

namespace groove_math {

    class AverageFunction {

    public:
        AverageFunction();

        MathStatus apply(double &output, const double &input);

    private:
        double m_sum;
        double m_count;
    };
}

#endif // GROOVE_MATH_AVERAGE_FUNCTION_H