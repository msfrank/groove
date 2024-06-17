
#include <groove_units/internal/units_listener.h>

groove_units::internal::UnitsListener::UnitsListener(const ReferenceSystemManager *manager)
{
    m_manager = manager;
}

groove_units::internal::UnitsListener::UnitsListener(const UnitReferenceSystem &rs)
{
    m_manager = nullptr;
    m_rs = rs;
}

void
groove_units::internal::UnitsListener::exitRsSpec(UnitsParser::RsSpecContext *ctx)
{
    if (m_manager && ctx->Identifier()) {
        auto rsId = ctx->Identifier()->getText();
        if (m_manager->containsReferenceSystem(rsId)) {
            m_rs = m_manager->getReferenceSystem(rsId);
        } else {
            m_rs = UnitReferenceSystem();
        }
    }
}

void
groove_units::internal::UnitsListener::enterDenominator(
    UnitsParser::DenominatorContext *ctx)
{
    m_parseNumerator = false;
}

void
groove_units::internal::UnitsListener::exitListOfIdentifiers(
    UnitsParser::ListOfIdentifiersContext *ctx)
{
    DerivedUnit derivedUnit;

    // get the unit name and verify that it is a base unit
    auto numIdentifiers = ctx->Identifier().size();
    auto baseOrDerivedUnitName = ctx->Identifier(numIdentifiers - 1)->getText();
    if (m_rs.containsBaseUnit(baseOrDerivedUnitName)) {
        derivedUnit.name = baseOrDerivedUnitName;
        derivedUnit.units = { {baseOrDerivedUnitName, 1} };
        derivedUnit.base = 1;
        derivedUnit.exponent = 1;
    } else if (m_rs.containsDerivedUnit(baseOrDerivedUnitName)) {
        derivedUnit = m_rs.getDerivedUnit(baseOrDerivedUnitName).getOrDefault(DerivedUnit());
    } else {
        TU_UNREACHABLE();
    }

    // get each scale product
    for (size_t i = 0; i < numIdentifiers - 1; i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        auto scaleName = ctx->Identifier(i)->getText();
        TU_ASSERT(m_rs.containsScaleProduct(scaleName));
        auto scaleProduct = m_rs.getScaleProduct(scaleName).getOrDefault({"", 0, 0});
        if (derivedUnit.base == 1) {
            derivedUnit.base = scaleProduct.base;
            derivedUnit.exponent = scaleProduct.exponent;
        } else if (derivedUnit.base == scaleProduct.base) {
            derivedUnit.exponent += scaleProduct.exponent;
        } else {
            TU_UNREACHABLE();
        }
    }

    if (m_parseNumerator) {
        // apply the scale product
        if (m_base == 1) {
            m_base = derivedUnit.base;
            m_exponent = derivedUnit.exponent;
        } else if (m_base == derivedUnit.base) {
            m_exponent+= derivedUnit.exponent;
        } else if (derivedUnit.base != 1) {
            TU_UNREACHABLE();
        }
        // apply the units
        for (auto iterator = derivedUnit.units.cbegin(); iterator != derivedUnit.units.cend(); iterator++) {
            const auto &unitName = iterator->first;
            const auto &unitExponent = iterator->second;
            if (m_units.contains(unitName)) {
                m_units[unitName] = m_units[unitName] + unitExponent;
            } else {
                m_units[unitName] = unitExponent;
            }
        }
    } else {
        // apply the negative of the scale product
        if (m_base == 1) {
            m_base = derivedUnit.base;
            m_exponent = -derivedUnit.exponent;
        } else if (m_base == derivedUnit.base) {
            m_exponent -= derivedUnit.exponent;
        } else if (derivedUnit.base != 1) {
            TU_UNREACHABLE();
        }
        // apply the inverse of the unit
        for (auto iterator = derivedUnit.units.cbegin(); iterator != derivedUnit.units.cend(); iterator++) {
            const auto &unitName = iterator->first;
            const auto &unitExponent = iterator->second;
            if (m_units.contains(unitName)) {
                m_units[unitName] = m_units[unitName] - unitExponent;
                if (m_units[unitName] == 0)
                    m_units.erase(unitName);
            } else {
                m_units[unitName] = -unitExponent;
            }
        }
    }
}

void
groove_units::internal::UnitsListener::exitListOfIdentifiersWithExponent(
    UnitsParser::ListOfIdentifiersWithExponentContext *ctx)
{
    int scaleBase = 1;
    int scaleExponent = 1;
    std::string unitName;
    int unitExponent;

    // get each scale product
    for (size_t i = 0; i < ctx->Identifier().size(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        auto scaleName = ctx->Identifier(i)->getText();
        TU_ASSERT(m_rs.containsScaleProduct(scaleName));
        auto scaleProduct = m_rs.getScaleProduct(scaleName).getOrDefault({"", 0, 0});
        if (scaleBase == 1) {
            scaleBase = scaleProduct.base;
            scaleExponent = scaleProduct.exponent;
        } else if (scaleBase == scaleProduct.base) {
            scaleExponent += scaleProduct.exponent;
        } else {
            TU_UNREACHABLE();
        }
    }

    // get the unit name and verify that it is a base unit
    unitName = ctx->unitAndExponent()->Identifier()->getText();
    TU_ASSERT(m_rs.containsBaseUnit(unitName));

    // get the unit exponent
    TU_ASSERT(absl::SimpleAtoi(ctx->unitAndExponent()->DecimalLiteral()->getText(), &unitExponent));

    if (m_parseNumerator) {
        // apply the scale product
        if (m_base == 1) {
            m_base = scaleBase;
            m_exponent = scaleExponent;
        } else if (m_base == scaleBase) {
            m_exponent+= scaleExponent;
        } else if (scaleBase != 1) {
            TU_UNREACHABLE();
        }
        // apply the unit
        if (m_units.contains(unitName)) {
            m_units[unitName] = m_units[unitName] + unitExponent;
        } else {
            m_units[unitName] = unitExponent;
        }
    } else {
        // apply the negative of the scale product
        if (m_base == 1) {
            m_base = scaleBase;
            m_exponent = -scaleExponent;
        } else if (m_base == scaleBase) {
            m_exponent -= scaleExponent;
        } else if (scaleBase != 1) {
            TU_UNREACHABLE();
        }
        // apply the inverse of the unit
        if (m_units.contains(unitName)) {
            m_units[unitName] = m_units[unitName] - unitExponent;
            if (m_units[unitName] == 0)
                m_units.erase(unitName);
        } else {
            m_units[unitName] = -unitExponent;
        }
    }
}

groove_units::UnitReferenceSystem
groove_units::internal::UnitsListener::getReferenceSystem() const
{
    return m_rs;
}

absl::flat_hash_map<std::string,int>
groove_units::internal::UnitsListener::getUnits() const
{
    return m_units;
}

int
groove_units::internal::UnitsListener::getScaleBase() const
{
    return m_base;
}

int
groove_units::internal::UnitsListener::getScaleExponent() const
{
    return m_exponent;
}
