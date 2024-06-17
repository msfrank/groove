
#include <groove_data/comparison_utils.h>
#include <groove_model/variant_key.h>

groove_model::VariantKey::VariantKey()
    : m_data(VariantKey::KeyType::kNil)
{
}

groove_model::VariantKey::VariantKey(const groove_data::Category &value)
    : m_data(value)
{
}

groove_model::VariantKey::VariantKey(double value)
    : m_data(value)
{
}

groove_model::VariantKey::VariantKey(tu_int64 value)
    : m_data(value)
{
}

groove_model::VariantKey::VariantKey(const VariantKey &other)
    : m_data(other.m_data)
{
}

bool
groove_model::VariantKey::isNil() const
{
    return getKeyType() == KeyType::kNil;
}

class get_key_type : public boost::static_visitor<groove_model::VariantKey::KeyType> {
public:
    groove_model::VariantKey::KeyType
    operator()(const groove_model::VariantKey::KeyType &v) const { return groove_model::VariantKey::KeyType::kNil; }

    groove_model::VariantKey::KeyType
    operator()(const groove_data::Category &v) const { return groove_model::VariantKey::KeyType::kCategory; }

    groove_model::VariantKey::KeyType
    operator()(const double &v) const { return groove_model::VariantKey::KeyType::kDouble; }

    groove_model::VariantKey::KeyType
    operator()(const tu_int64 &v) const { return groove_model::VariantKey::KeyType::kInt64; }
};

groove_model::VariantKey::KeyType
groove_model::VariantKey::getKeyType() const
{
    return boost::apply_visitor(get_key_type(), m_data);
}

groove_data::Category
groove_model::VariantKey::getCategory() const
{
    auto *cat = boost::get<groove_data::Category>(&m_data);
    TU_ASSERT (cat != nullptr);
    return *cat;
}

Option<groove_data::Category>
groove_model::VariantKey::categoryOption() const
{
    auto *cat = boost::get<groove_data::Category>(&m_data);
    if (cat != nullptr)
        return Option<groove_data::Category>(*cat);
    return Option<groove_data::Category>();
}

double
groove_model::VariantKey::getDouble() const
{
    auto *dbl = boost::get<double>(&m_data);
    TU_ASSERT (dbl != nullptr);
    return *dbl;
}

Option<double>
groove_model::VariantKey::doubleOption() const
{
    auto *dbl = boost::get<double>(&m_data);
    if (dbl != nullptr)
        return Option<double>(*dbl);
    return Option<double>();
}

tu_int64
groove_model::VariantKey::getInt64() const
{
    auto *i64 = boost::get<tu_int64>(&m_data);
    TU_ASSERT (i64 != nullptr);
    return *i64;
}

Option<tu_int64>
groove_model::VariantKey::int64Option() const
{
    auto *i64 = boost::get<tu_int64>(&m_data);
    if (i64 != nullptr)
        return Option<tu_int64>(*i64);
    return Option<tu_int64>();
}

class is_equal : public boost::static_visitor<bool> {
public:
    template <typename T, typename U>
    bool operator()(const T &lhs, const U &rhs) const { return false; }

    template <typename T>
    bool operator()(const T &lhs, const T &rhs) const { return lhs == rhs; }

};

bool
groove_model::VariantKey::operator==(const VariantKey &other) const
{
    return boost::apply_visitor(is_equal(), m_data, other.m_data);
}

bool
groove_model::VariantKey::operator!=(const VariantKey &other) const
{
    return !boost::apply_visitor(is_equal(), m_data, other.m_data);
}

class compare : public boost::static_visitor<Option<int>> {
public:
    template <typename T, typename U>
    Option<int> operator()(const T &lhs, const U &rhs) const { return Option<int>(); }

    Option<int>
    operator()(const groove_model::VariantKey::KeyType &lhs, const groove_model::VariantKey::KeyType &rhs) const {
        return Option<int>(0);
    }

    template <typename T>
    Option<int> operator()(const T &lhs, const T &rhs) const { return Option<int>(groove_data::compare_key(lhs,rhs)); }

};

bool
groove_model::VariantKey::operator<(const VariantKey &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() < 0;
}

bool
groove_model::VariantKey::operator<=(const VariantKey &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() <= 0;
}

bool
groove_model::VariantKey::operator>(const VariantKey &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() > 0;
}

bool
groove_model::VariantKey::operator>=(const VariantKey &other) const
{
    auto ret = boost::apply_visitor(compare(), m_data, other.m_data);
    if (ret.isEmpty())
        return false;
    return ret.getValue() >= 0;
}