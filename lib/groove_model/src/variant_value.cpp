
#include <groove_data/comparison_utils.h>
#include <groove_model/variant_value.h>

groove_model::VariantValue::VariantValue()
    : m_data(VariantValue::ValueType::kNil)
{
}

groove_model::VariantValue::VariantValue(double value)
    : m_data(value)
{
}

groove_model::VariantValue::VariantValue(tu_int64 value)
    : m_data(value)
{
}

groove_model::VariantValue::VariantValue(const std::string &value)
    : m_data(value)
{
}

groove_model::VariantValue::VariantValue(const VariantValue &other)
    : m_data(other.m_data)
{
}

bool
groove_model::VariantValue::isNil() const
{
    return getValueType() == ValueType::kNil;
}

class get_key_type : public boost::static_visitor<groove_model::VariantValue::ValueType> {
public:
    groove_model::VariantValue::ValueType
    operator()(const groove_model::VariantValue::ValueType &v) const { return groove_model::VariantValue::ValueType::kNil; }

    groove_model::VariantValue::ValueType
    operator()(const double &v) const { return groove_model::VariantValue::ValueType::kDouble; }

    groove_model::VariantValue::ValueType
    operator()(const tu_int64 &v) const { return groove_model::VariantValue::ValueType::kInt64; }

    groove_model::VariantValue::ValueType
    operator()(const std::string &v) const { return groove_model::VariantValue::ValueType::kString; }
};

groove_model::VariantValue::ValueType
groove_model::VariantValue::getValueType() const
{
    return boost::apply_visitor(get_key_type(), m_data);
}

double
groove_model::VariantValue::getDouble() const
{
    auto *dbl = boost::get<double>(&m_data);
    TU_ASSERT (dbl != nullptr);
    return *dbl;
}

Option<double>
groove_model::VariantValue::doubleOption() const
{
    auto *dbl = boost::get<double>(&m_data);
    if (dbl != nullptr)
        return Option<double>(*dbl);
    return Option<double>();
}

tu_int64
groove_model::VariantValue::getInt64() const
{
    auto *i64 = boost::get<tu_int64>(&m_data);
    TU_ASSERT (i64 != nullptr);
    return *i64;
}

Option<tu_int64>
groove_model::VariantValue::int64Option() const
{
    auto *i64 = boost::get<tu_int64>(&m_data);
    if (i64 != nullptr)
        return Option<tu_int64>(*i64);
    return Option<tu_int64>();
}

std::string
groove_model::VariantValue::getString() const
{
    auto *str = boost::get<std::string>(&m_data);
    TU_ASSERT (str != nullptr);
    return *str;
}

Option<std::string>
groove_model::VariantValue::stringOption() const
{
    auto *str = boost::get<std::string>(&m_data);
    if (str != nullptr)
        return Option<std::string>(*str);
    return Option<std::string>();
}

class is_equal : public boost::static_visitor<bool> {
public:
    template <typename T, typename U>
    bool operator()(const T &lhs, const U &rhs) const { return false; }

    template <typename T>
    bool operator()(const T &lhs, const T &rhs) const { return lhs == rhs; }

};

bool
groove_model::VariantValue::operator==(const VariantValue &other) const
{
    return boost::apply_visitor(is_equal(), m_data, other.m_data);
}

bool
groove_model::VariantValue::operator!=(const VariantValue &other) const
{
    return !boost::apply_visitor(is_equal(), m_data, other.m_data);
}

class compare : public boost::static_visitor<Option<int>> {
public:
    template <typename T, typename U>
    Option<int> operator()(const T &lhs, const U &rhs) const { return Option<int>(); }

    Option<int>
    operator()(const groove_model::VariantValue::ValueType &lhs, const groove_model::VariantValue::ValueType &rhs) const {
        return Option<int>(0);
    }

    template <typename T>
    Option<int> operator()(const T &lhs, const T &rhs) const { return Option<int>(groove_data::compare_value(lhs,rhs)); }

};

bool
groove_model::VariantValue::operator<(const VariantValue &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() < 0;
}

bool
groove_model::VariantValue::operator<=(const VariantValue &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() <= 0;
}

bool
groove_model::VariantValue::operator>(const VariantValue &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() > 0;
}

bool
groove_model::VariantValue::operator>=(const VariantValue &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() >= 0;
}
