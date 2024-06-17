#ifndef GROOVE_MATH_MAXIMUM_FUNCTION_H
#define GROOVE_MATH_MAXIMUM_FUNCTION_H

#include <tempo_utils/option_template.h>

#include "math_result.h"

namespace groove_math {

    class MaximumFunction {

    public:
        MaximumFunction();

        MathStatus apply(double &output, const double &input);

    private:
        Option<double> m_max;
    };
}

#endif // GROOVE_MATH_MAXIMUM_FUNCTION_H