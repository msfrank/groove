#ifndef GROOVE_UNITS_UNIT_SCALE_H
#define GROOVE_UNITS_UNIT_SCALE_H

#include <cstdint>
#include <string>

#include "units_result.h"

namespace groove_units {

    struct UnitScalePriv {
        std::string name;
        int64_t product;
    };

    class UnitScale {
    public:
        UnitScale();
        UnitScale(std::string_view name, int64_t product);
        UnitScale(const UnitScale &other);

        bool isValid() const;

        std::string getName() const;
        int64_t getProduct() const;

        std::string toString() const;

        bool operator==(const UnitScale &other) const;
        bool operator!=(const UnitScale &other) const;

        static tempo_utils::Result<UnitScale> parse(std::string_view scale);

    private:
        std::shared_ptr<UnitScalePriv> m_priv;
    };
}

#endif // GROOVE_UNITS_UNIT_SCALE_H
