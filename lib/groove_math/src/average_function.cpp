
#include <groove_math/average_function.h>

groove_math::AverageFunction::AverageFunction()
    : m_sum(0.0),
      m_count(0.0)
{
}

groove_math::MathStatus
groove_math::AverageFunction::apply(double &output, const double &input)
{
    m_sum += input;
    m_count++;
    output = m_sum / m_count;
    return MathStatus::ok();
}