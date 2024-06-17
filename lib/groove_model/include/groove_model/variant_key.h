#ifndef GROOVE_MODEL_VARIANT_KEY_H
#define GROOVE_MODEL_VARIANT_KEY_H

#include <boost/variant.hpp>

#include <groove_data/category.h>
#include <tempo_utils/option_template.h>

namespace groove_model {

    class VariantKey {

    public:

        enum class KeyType {
            kNil = 0,
            kCategory = 1,
            kDouble = 2,
            kInt64 = 3,
        };

        VariantKey();
        explicit VariantKey(const groove_data::Category &value);
        explicit VariantKey(double value);
        explicit VariantKey(tu_int64 value);
        VariantKey(const VariantKey &other);

        bool isNil() const;

        KeyType getKeyType() const;

        groove_data::Category getCategory() const;
        Option<groove_data::Category> categoryOption() const;
        double getDouble() const;
        Option<double> doubleOption() const;
        tu_int64 getInt64() const;
        Option<tu_int64> int64Option() const;

        bool operator==(const VariantKey &other) const;
        bool operator!=(const VariantKey &other) const;
        bool operator<(const VariantKey &other) const;
        bool operator<=(const VariantKey &other) const;
        bool operator>(const VariantKey &other) const;
        bool operator>=(const VariantKey &other) const;

    protected:
        boost::variant <KeyType, groove_data::Category, double, tu_int64> m_data;
    };
}

#endif // GROOVE_MODEL_VARIANT_KEY_H