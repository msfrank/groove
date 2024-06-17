#include <iostream>
#include <string>

#include <absl/strings/str_join.h>
#include <absl/strings/substitute.h>

#include <groove_units/internal/units_listener.h>
#include <groove_units/unicode_utils.h>
#include <groove_units/unit_reference_system.h>
#include <tempo_utils/log_stream.h>

#include "antlr4-runtime.h"
#include "UnitsLexer.h"
#include "UnitsParser.h"

static inline std::string
scale_product_id(int base, int exponent)
{
    return absl::Substitute("$0^$1", base, exponent);
}

static inline std::string
unit_dimension_id(std::string_view rs, const absl::flat_hash_map<std::string,int> &units, int base, int exponent)
{
    std::vector<std::string> unitsList;
    for (const auto &unit : units) {
        unitsList.push_back(absl::StrCat(unit.first, unit.second));
    }
    std::sort(unitsList.begin(), unitsList.end());

    return absl::StrCat(
        rs, ":",
        scale_product_id(base, exponent), ":",
        absl::StrJoin(unitsList, ""));
}

groove_units::UnitReferenceSystem::UnitReferenceSystem()
{
}

groove_units::UnitReferenceSystem::UnitReferenceSystem(
    std::string_view name,
    const absl::flat_hash_set<std::string> &baseUnits,
    const std::vector<ScaleProduct> &scaleProducts)
{
    m_name = name;
    m_baseUnits = baseUnits;
    for (const auto &scaleProduct : scaleProducts) {
        auto scaleProductId = scale_product_id(scaleProduct.base, scaleProduct.exponent);
        if (!m_scaleProducts.contains(scaleProductId)) {
            m_scaleProducts[scaleProductId] = scaleProduct;
            m_nameToScaleProduct[scaleProduct.name] = scaleProductId;
        } else {
            TU_LOG_WARN << this << "ignoring duplicate scale product " << scaleProduct.name;
        }
    }
}

groove_units::UnitReferenceSystem::UnitReferenceSystem(const UnitReferenceSystem &other)
    : m_name(other.m_name),
      m_baseUnits(other.m_baseUnits),
      m_scaleProducts(other.m_scaleProducts),
      m_derivedUnits(other.m_derivedUnits),
      m_nameToDerivedUnits(other.m_nameToDerivedUnits),
      m_nameToScaleProduct(other.m_nameToScaleProduct)
{
}

bool
groove_units::UnitReferenceSystem::containsBaseUnit(std::string_view name) const
{
    return m_baseUnits.contains(name);
}

std::vector<std::string>
groove_units::UnitReferenceSystem::listBaseUnits() const
{
    return std::vector<std::string>(m_baseUnits.cbegin(), m_baseUnits.cend());
}

bool
groove_units::UnitReferenceSystem::containsScaleProduct(std::string_view name) const
{
    return m_nameToScaleProduct.contains(name);
}

Option<groove_units::ScaleProduct>
groove_units::UnitReferenceSystem::getScaleProduct(std::string_view name) const
{
    if (!m_nameToScaleProduct.contains(name))
        return Option<ScaleProduct>();
    auto scaleId = m_nameToScaleProduct.at(name);
    return Option<ScaleProduct>(m_scaleProducts.at(scaleId));
}

std::vector<std::string>
groove_units::UnitReferenceSystem::listScaleProducts() const
{
    std::vector<std::string> scaleProducts;
    for (const auto &nameToScale : m_nameToScaleProduct) {
        scaleProducts.push_back(nameToScale.first);
    }
    return scaleProducts;
}

bool
groove_units::UnitReferenceSystem::containsDerivedUnit(std::string_view name) const
{
    return m_nameToDerivedUnits.contains(name);
}

Option<groove_units::DerivedUnit>
groove_units::UnitReferenceSystem::getDerivedUnit(std::string_view name) const
{
    if (!m_nameToDerivedUnits.contains(name))
        return Option<DerivedUnit>();
    auto derivedId = m_nameToDerivedUnits.at(name);
    return Option<DerivedUnit>(m_derivedUnits.at(derivedId));
}

groove_units::UnitsStatus
groove_units::UnitReferenceSystem::setDerivedUnit(
    std::string_view name,
    const absl::flat_hash_map<std::string,int> &units,
    int base,
    int exponent)
{
    if (m_baseUnits.contains(name))
        return UnitsStatus::forCondition(UnitsCondition::kMissingBaseUnit);
    if (m_scaleProducts.contains(name))
        return UnitsStatus::forCondition(UnitsCondition::kMissingScaleProduct);

    auto derivedUnitId = unit_dimension_id(m_name, units, base, exponent);
    m_derivedUnits[derivedUnitId] = {std::string(name), units, base, exponent};
    m_nameToDerivedUnits[name] = derivedUnitId;
    return UnitsStatus::ok();
}

void
groove_units::UnitReferenceSystem::removeDerivedUnit(std::string_view name)
{
    if (!m_nameToDerivedUnits.contains(name))
        return;
    auto derivedId = m_nameToDerivedUnits[name];
    m_derivedUnits.erase(derivedId);
    m_nameToDerivedUnits.erase(name);
}

std::vector<std::string>
groove_units::UnitReferenceSystem::listDerivedUnits() const
{
    std::vector<std::string> derivedUnits;
    for (const auto &nameToDerived : m_nameToDerivedUnits) {
        derivedUnits.push_back(nameToDerived.first);
    }
    return derivedUnits;
}

tempo_utils::Result<groove_units::UnitDimension>
groove_units::UnitReferenceSystem::parse(const absl::flat_hash_map<std::string,int> &units, int base, int exponent) const
{
//    std::vector<std::string> unitsList;
//    for (const auto &unit : units) {
//        unitsList.push_back(absl::StrCat(unit.first, unit.second));
//    }
//    std::sort(unitsList.begin(), unitsList.end());
//    std::string unitsString = absl::StrJoin(unitsList, "");
//
//    std::string scaleProductString;
//    if (base != 1) {
//        scaleProductString = absl::StrCat(base, "^", exponent);
//    }

    std::string unitDimensionId = unit_dimension_id(m_name, units, base, exponent);

    if (m_derivedUnits.contains(unitDimensionId)) {
        const auto &derivedUnit = m_derivedUnits.at(unitDimensionId);
        return tempo_utils::Result<UnitDimension>(
            UnitDimension(unitDimensionId, derivedUnit.name, units, base, exponent));
    }

    std::string derivedUnitId = unit_dimension_id(m_name, units, 1, 1);
    std::string scaleProductId = scale_product_id(base, exponent);
    std::string name;

    if (m_derivedUnits.contains(derivedUnitId) && m_scaleProducts.contains(scaleProductId)) {
        const auto &derivedUnit = m_derivedUnits.at(derivedUnitId);
        const auto &scaleProduct = m_scaleProducts.at(scaleProductId);
        name = absl::StrCat(scaleProduct.name, " ", derivedUnit.name);
    } else if (m_derivedUnits.contains(derivedUnitId)) {
        const auto &derivedUnit = m_derivedUnits.at(derivedUnitId);
        if (base > 1) {
            absl::flat_hash_map<std::string,int> scaleProduct = { {absl::StrCat(base), exponent} };
            name = absl::StrCat(pretty_print_units(scaleProduct), " ", derivedUnit.name);
        } else {
            name = derivedUnit.name;
        }
    } else if (m_scaleProducts.contains(scaleProductId)) {
        const auto &scaleProduct = m_scaleProducts.at(scaleProductId);
        name = absl::StrCat(scaleProduct.name, " ", pretty_print_units(units));
    } else {
        if (base > 1) {
            absl::flat_hash_map<std::string,int> scaleProduct = { {absl::StrCat(base), exponent} };
            name = absl::StrCat(pretty_print_units(scaleProduct), " ", pretty_print_units(units));
        } else {
            name = pretty_print_units(units);
        }
    }

    return tempo_utils::Result<UnitDimension>(
        UnitDimension(unitDimensionId, name, units, base, exponent));
}

tempo_utils::Result<groove_units::UnitDimension>
groove_units::UnitReferenceSystem::parse(std::string_view utf8) const
{
    if (utf8.empty())
        return tempo_utils::Result<UnitDimension>(UnitDimension());

    antlr4::ANTLRInputStream input(utf8.data(), (size_t) utf8.size());
    UnitsLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    UnitsParser parser(&tokens);

    internal::UnitsListener listener(*this);

    antlr4::tree::ParseTree *tree = parser.units();
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    return parse(listener.getUnits(), listener.getScaleBase(), listener.getScaleExponent());
}
