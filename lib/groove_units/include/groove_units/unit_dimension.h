#ifndef GROOVE_UNITS_UNIT_DIMENSION_H
#define GROOVE_UNITS_UNIT_DIMENSION_H

#include <string>

#include <absl/container/flat_hash_map.h>

#include "unit_scale.h"
#include "units_result.h"

namespace groove_units {

    struct UnitDimensionPriv {
        std::string id;
        std::string name;
        absl::flat_hash_map<std::string, int> units;
        int base;
        int exponent;
    };

    class UnitDimension {
    public:
        UnitDimension();
        UnitDimension(
            std::string_view id,
            std::string_view name,
            const absl::flat_hash_map<std::string, int> &units,
            int base,
            int exponent);
        UnitDimension(const UnitDimension &other);

        bool isValid() const;

        std::string getId() const;
        std::string getName() const;
        int getBase() const;
        int getExponent() const;

        std::vector<std::string> listUnitNames() const;
        bool containsUnit(std::string_view name) const;
        int getUnit(std::string_view name) const;

        UnitsStatus scale(const UnitDimension &target, int *base, int *exponent);

        std::string toString() const;

        bool operator==(const UnitDimension &other) const;
        bool operator!=(const UnitDimension &other) const;

    private:
        std::shared_ptr<UnitDimensionPriv> m_priv;
    };

    std::string units_to_string(const absl::flat_hash_map<std::string, int> &units);
    std::string units_to_pretty_string(const absl::flat_hash_map<std::string, int> &units);
    tempo_utils::Result<absl::flat_hash_map<std::string, int>> string_to_base_units_map(std::string_view string);
    std::string exponent_to_unicode_superscript(int exp);
}

#endif // GROOVE_UNITS_UNIT_DIMENSION_H
