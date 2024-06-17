
#include <groove_math/maximum_function.h>

groove_math::MaximumFunction::MaximumFunction()
    : m_max()
{
}

groove_math::MathStatus
groove_math::MaximumFunction::apply(double &output, const double &input)
{
    if (m_max.isEmpty() || m_max.getValue() < input) {
        m_max = Option<double>(input);
    }
    output = m_max.getValue();
    return MathStatus::ok();
}