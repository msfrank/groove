
#include <groove_math/minimum_function.h>

groove_math::MinimumFunction::MinimumFunction()
    : m_min()
{
}

groove_math::MathStatus
groove_math::MinimumFunction::apply(double &output, const double &input)
{
    if (m_min.isEmpty() || m_min.getValue() > input) {
        m_min = Option<double>(input);
    }
    output = m_min.getValue();
    return MathStatus::ok();
}