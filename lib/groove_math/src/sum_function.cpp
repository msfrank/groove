
#include <groove_math/sum_function.h>

groove_math::SumFunction::SumFunction()
    : m_sum(0.0)
{
}

groove_math::MathStatus
groove_math::SumFunction::apply(double &output, const double &input)
{
    m_sum += input;
    output = m_sum;
    return MathStatus::ok();
}