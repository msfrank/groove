#ifndef GROOVE_UNITS_REFERENCE_SYSTEM_MANAGER_H
#define GROOVE_UNITS_REFERENCE_SYSTEM_MANAGER_H

#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/strings/string_view.h>

#include "unit_reference_system.h"

namespace groove_units {

    class ReferenceSystemManager {
    public:
        ReferenceSystemManager();

        bool containsReferenceSystem(std::string_view name) const;
        UnitReferenceSystem getReferenceSystem(std::string_view name) const;
        void setReferenceSystem(std::string_view name, const UnitReferenceSystem &rs);
        void removeReferenceSystem(std::string_view name);
        std::vector<std::string> listReferenceSystems() const;

        tempo_utils::Result<UnitDimension> parse(std::string_view utf8) const;

    private:
        absl::flat_hash_map<std::string,UnitReferenceSystem> m_systems;
    };
}

#endif // GROOVE_UNITS_REFERENCE_SYSTEM_MANAGER_H
