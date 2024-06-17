#ifndef GROOVE_UNITS_INTERNAL_UNITS_LISTENER_H
#define GROOVE_UNITS_INTERNAL_UNITS_LISTENER_H

#include <string>
#include <absl/container/flat_hash_map.h>

#include <groove_units/reference_system_manager.h>
#include <groove_units/unit_reference_system.h>

#include "UnitsParserBaseListener.h"

namespace groove_units::internal {

    class UnitsListener : public UnitsParserBaseListener {

    public:
        explicit UnitsListener(const ReferenceSystemManager *manager);
        explicit UnitsListener(const UnitReferenceSystem &rs);

        void exitRsSpec(UnitsParser::RsSpecContext *ctx) override;
        void enterDenominator(UnitsParser::DenominatorContext *ctx) override;
        void exitListOfIdentifiers(UnitsParser::ListOfIdentifiersContext *ctx) override;
        void exitListOfIdentifiersWithExponent(UnitsParser::ListOfIdentifiersWithExponentContext *ctx) override;

        UnitReferenceSystem getReferenceSystem() const;
        absl::flat_hash_map<std::string, int> getUnits() const;
        int getScaleBase() const;
        int getScaleExponent() const;

    private:
        const ReferenceSystemManager *m_manager;
        UnitReferenceSystem m_rs;
        absl::flat_hash_map<std::string, int> m_units;
        int m_base = 1;
        int m_exponent = 1;
        bool m_parseNumerator = true;
    };
}

#endif // GROOVE_UNITS_INTERNAL_UNITS_LISTENER_H
