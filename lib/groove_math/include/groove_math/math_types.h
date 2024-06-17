#ifndef GROOVE_MATH_MATH_TYPES_H
#define GROOVE_MATH_MATH_TYPES_H

namespace groove_math {

    enum class ExpressionType {
        EXPRESSION_UNKNOWN,
        EXPRESSION_AVG,
        EXPRESSION_MAX,
        EXPRESSION_MIN,
        EXPRESSION_SAMPLECOUNT,
        EXPRESSION_SUM
    };

    class ReduceDef {

    public:
        ReduceDef();
        ReduceDef(ExpressionType expressionType);
        ReduceDef(const ReduceDef &other);

        ExpressionType getExpressionType() const;

    private:
        ExpressionType m_expressionType;
    };

    template <typename DefType>
    struct ReducerTraits {};

    struct Average : public ReduceDef {
        Average();
    };

    struct Minimum : public ReduceDef {
        Minimum();
    };

    struct Maximum : public ReduceDef {
        Maximum();
    };

    struct Sum : public ReduceDef {
        Sum();
    };

    struct SampleCount : public ReduceDef {
        SampleCount();
    };
}

#endif // GROOVE_MATH_MATH_TYPES_H