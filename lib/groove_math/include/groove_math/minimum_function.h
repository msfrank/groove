#ifndef GROOVE_MATH_MINIMUM_FUNCTION_H
#define GROOVE_MATH_MINIMUM_FUNCTION_H

#include <tempo_utils/option_template.h>

#include "math_result.h"

namespace groove_math {

    class MinimumFunction {

    public:
        MinimumFunction();

        MathStatus apply(double &output, const double &input);

    private:
        Option<double> m_min;
    };
}

#endif // GROOVE_MATH_MINIMUM_FUNCTION_H