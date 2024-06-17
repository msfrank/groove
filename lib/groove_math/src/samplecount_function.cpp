
#include <groove_math/samplecount_function.h>

groove_math::SampleCountFunction::SampleCountFunction()
    : m_count(0.0)
{
}

groove_math::MathStatus
groove_math::SampleCountFunction::apply(double &output, const double &input)
{
    m_count++;
    output = m_count;
    return MathStatus::ok();
}