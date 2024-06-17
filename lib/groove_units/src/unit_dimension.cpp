
#include <groove_units/unit_dimension.h>

groove_units::UnitDimension::UnitDimension()
{
    m_priv = std::make_shared<UnitDimensionPriv>();
    m_priv->base = 0;
    m_priv->exponent = 0;
}

groove_units::UnitDimension::UnitDimension(
    std::string_view id,
    std::string_view name,
    const absl::flat_hash_map<std::string,int> &units,
    int base,
    int exponent)
{
    m_priv = std::make_shared<UnitDimensionPriv>();
    m_priv->id = id;
    m_priv->name = name;
    m_priv->units = units;
    m_priv->base = base;
    m_priv->exponent = exponent;
}

groove_units::UnitDimension::UnitDimension(const UnitDimension &other)
    : m_priv(other.m_priv)
{
}

bool
groove_units::UnitDimension::isValid() const
{
    return !m_priv->units.empty();
}

std::string
groove_units::UnitDimension::getId() const
{
    return m_priv->id;
}

std::string
groove_units::UnitDimension::getName() const
{
    return m_priv->name;
}

int
groove_units::UnitDimension::getBase() const
{
    return m_priv->base;
}

int
groove_units::UnitDimension::getExponent() const
{
    return m_priv->exponent;
}

std::vector<std::string>
groove_units::UnitDimension::listUnitNames() const
{
    std::vector<std::string> unitNames;
    for (const auto &unit : m_priv->units) {
        unitNames.push_back(unit.first);
    }
    return unitNames;
}

bool
groove_units::UnitDimension::containsUnit(std::string_view name) const
{
    return m_priv->units.contains(name);
}

int
groove_units::UnitDimension::getUnit(std::string_view name) const
{
    if (m_priv->units.contains(name))
        return m_priv->units.at(name);
    return 0;
}

groove_units::UnitsStatus
groove_units::UnitDimension::scale(
    const UnitDimension &target,
    int *base,
    int *exponent)
{
    auto fixedUnitNames = listUnitNames();
    auto targetUnitNames = target.listUnitNames();
    if (fixedUnitNames != targetUnitNames)
        return UnitsStatus::forCondition(UnitsCondition::kUnitDimensionConflict);

    for (const auto &unitName : fixedUnitNames) {
        if (getUnit(unitName) != target.getUnit(unitName))
            return UnitsStatus::forCondition(UnitsCondition::kUnitExponentConflict);
    }

    auto fixedBase = getBase();
    auto targetBase = target.getBase();
    if (fixedBase != targetBase && fixedBase != 1 && targetBase != 1)
        return UnitsStatus::forCondition(UnitsCondition::kScaleProductBaseConflict);

    auto fixedExponent = fixedBase == 1 ? 0 : getExponent();
    auto targetExponent = targetBase == 1 ? 0 : target.getExponent();

    if (base != nullptr)
        *base = fixedBase != 1 ? fixedBase : targetBase;
    if (exponent != nullptr) {
        *exponent = abs(fixedExponent - targetExponent);
        if (targetExponent < fixedExponent)
            *exponent = -(*exponent);
    }
    return UnitsStatus::ok();
}

std::string
groove_units::UnitDimension::toString() const
{
    return getName();
}

bool
groove_units::UnitDimension::operator==(const UnitDimension &other) const
{
    return m_priv->id == other.m_priv->id
        && m_priv->name == other.m_priv->name
        && m_priv->units == other.m_priv->units
        && m_priv->base == other.m_priv->base
        && m_priv->exponent == other.m_priv->exponent;
}

bool
groove_units::UnitDimension::operator!=(const UnitDimension &other) const
{
    return !(this->operator==(other));
}