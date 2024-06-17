
#include <groove_units/unit_scale.h>

groove_units::UnitScale::UnitScale()
{
    m_priv = std::make_shared<UnitScalePriv>();
    m_priv->product = 0;
}

groove_units::UnitScale::UnitScale(std::string_view name, int64_t product)
{
    m_priv = std::make_shared<UnitScalePriv>();
    m_priv->name = name;
    m_priv->product = product;
}

groove_units::UnitScale::UnitScale(const UnitScale &other)
    : m_priv(other.m_priv)
{
}

bool
groove_units::UnitScale::isValid() const
{
    return m_priv->product != 0;
}

std::string
groove_units::UnitScale::getName() const
{
    return m_priv->name;
}

int64_t
groove_units::UnitScale::getProduct() const
{
    return m_priv->product;
}

std::string
groove_units::UnitScale::toString() const
{
    return "";
}

bool
groove_units::UnitScale::operator==(const UnitScale &other) const
{
    return m_priv->product == other.m_priv->product && m_priv->name == other.m_priv->name;
}

bool
groove_units::UnitScale::operator!=(const UnitScale &other) const
{
    return !(this->operator==(other));
}

tempo_utils::Result<groove_units::UnitScale>
groove_units::UnitScale::parse(std::string_view scale)
{
    return UnitsStatus::forCondition(
        UnitsCondition::kUnitsInvariant, "UnitScale::parse unimplemented");
}