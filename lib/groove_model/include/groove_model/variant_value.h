#ifndef GROOVE_MODEL_VARIANT_VALUE_H
#define GROOVE_MODEL_VARIANT_VALUE_H

#include <boost/variant.hpp>

#include <groove_data/category.h>
#include <tempo_utils/option_template.h>

namespace groove_model {

    class VariantValue {

    public:

        enum class ValueType {
            kNil = 0,
            kDouble = 1,
            kInt64 = 2,
            kString = 3,
        };

        VariantValue();
        explicit VariantValue(double value);
        explicit VariantValue(tu_int64 value);
        explicit VariantValue(const std::string &value);
        VariantValue(const VariantValue &other);

        bool isNil() const;

        ValueType getValueType() const;

        double getDouble() const;
        Option<double> doubleOption() const;
        tu_int64 getInt64() const;
        Option<tu_int64> int64Option() const;
        std::string getString() const;
        Option<std::string> stringOption() const;

        bool operator==(const VariantValue &other) const;
        bool operator!=(const VariantValue &other) const;
        bool operator<(const VariantValue &other) const;
        bool operator<=(const VariantValue &other) const;
        bool operator>(const VariantValue &other) const;
        bool operator>=(const VariantValue &other) const;

    protected:
        boost::variant <ValueType, double, tu_int64, std::string> m_data;
    };
}

#endif // GROOVE_MODEL_VARIANT_VALUE_H