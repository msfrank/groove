#ifndef GROOVE_UNITS_UNIT_REFERENCE_SYSTEM_H
#define GROOVE_UNITS_UNIT_REFERENCE_SYSTEM_H

#include <tempo_utils/option_template.h>

#include "unit_dimension.h"
#include "unit_scale.h"
#include "units_result.h"

namespace groove_units {

    struct ScaleProduct {
        std::string name;
        int base;
        int exponent;
    };

    struct DerivedUnit {
        std::string name;
        absl::flat_hash_map<std::string, int> units;
        int base;
        int exponent;
    };

    class UnitReferenceSystem {
    public:
        UnitReferenceSystem();
        UnitReferenceSystem(
            std::string_view name,
            const absl::flat_hash_set<std::string> &baseUnits,
            const std::vector<ScaleProduct> &scaleProducts);
        UnitReferenceSystem(const UnitReferenceSystem &other);

        bool containsBaseUnit(std::string_view name) const;
        std::vector<std::string> listBaseUnits() const;

        bool containsScaleProduct(std::string_view name) const;
        Option<ScaleProduct> getScaleProduct(std::string_view name) const;
        std::vector<std::string> listScaleProducts() const;

        bool containsDerivedUnit(std::string_view name) const;
        Option<DerivedUnit> getDerivedUnit(std::string_view name) const;
        UnitsStatus setDerivedUnit(
            std::string_view name,
            const absl::flat_hash_map<std::string, int> &units,
            int base = 1,
            int exponent = 1);
        void removeDerivedUnit(std::string_view name);
        std::vector<std::string> listDerivedUnits() const;

        tempo_utils::Result<UnitDimension> parse(
            const absl::flat_hash_map<std::string, int> &units,
            int base,
            int exponent) const;
        tempo_utils::Result<UnitDimension> parse(std::string_view string) const;

    private:
        std::string m_name;
        absl::flat_hash_set<std::string> m_baseUnits;                       // set of base unit ids
        absl::flat_hash_map<std::string, ScaleProduct> m_scaleProducts;     // map of id to scale product
        absl::flat_hash_map<std::string, DerivedUnit> m_derivedUnits;       // map of id to derived unit
        absl::flat_hash_map<std::string, std::string> m_nameToDerivedUnits; // map of name to derived unit id
        absl::flat_hash_map<std::string, std::string> m_nameToScaleProduct; // map of name to scale product id
    };

    std::string scale_product_to_string(int base, int exponent);

    tempo_utils::Result<ScaleProduct> string_to_scale_product(std::string_view string);
}

#endif // GROOVE_UNITS_UNIT_REFERENCE_SYSTEM_H
