#ifndef GROOVE_UNITS_MAGNITUDE_H
#define GROOVE_UNITS_MAGNITUDE_H

#include <cstdint>

#include "units_result.h"

namespace groove_units {

    enum class MagnitudeType {
        UNKNOWN,
        INTEGER,
        FLOAT,
        RATIO,
        SYMBOL
    };

    class Magnitude {

    public:
        Magnitude();
        explicit Magnitude(int64_t i64);
        explicit Magnitude(double dbl);
        Magnitude(int64_t numerator, int64_t denominator);

        bool isValid() const;

        MagnitudeType getType() const;

        static tempo_utils::Result<Magnitude> add(const Magnitude &lhs, const Magnitude &rhs);
        static tempo_utils::Result<Magnitude> subtract(const Magnitude &lhs, const Magnitude &rhs);
        static tempo_utils::Result<Magnitude> multiply(const Magnitude &lhs, const Magnitude &rhs);
        static tempo_utils::Result<Magnitude> divide(const Magnitude &lhs, const Magnitude &rhs);

    private:
        MagnitudeType m_type;
        union {
            int64_t i64;
            double dbl;
            struct {
                int64_t n;
                int64_t d;
            } ratio;
        } m_magnitude;
    };
}

#endif // GROOVE_UNITS_MAGNITUDE_H
