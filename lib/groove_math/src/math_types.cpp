
#include <groove_math/math_types.h>

groove_math::ReduceDef::ReduceDef()
    : m_expressionType(ExpressionType::EXPRESSION_UNKNOWN)
{
}

groove_math::ReduceDef::ReduceDef(ExpressionType expressionType)
    : m_expressionType(expressionType)
{
}

groove_math::ReduceDef::ReduceDef(const ReduceDef &other)
    : m_expressionType(other.m_expressionType)
{
}

groove_math::ExpressionType
groove_math::ReduceDef::getExpressionType() const
{
    return m_expressionType;
}

groove_math::Average::Average() : ReduceDef(ExpressionType::EXPRESSION_AVG)
{
}

groove_math::Minimum::Minimum() : ReduceDef(ExpressionType::EXPRESSION_MIN)
{
}

groove_math::Maximum::Maximum() : ReduceDef(ExpressionType::EXPRESSION_MAX)
{
}

groove_math::Sum::Sum() : ReduceDef(ExpressionType::EXPRESSION_SUM)
{
}

groove_math::SampleCount::SampleCount() : ReduceDef(ExpressionType::EXPRESSION_SAMPLECOUNT)
{
}