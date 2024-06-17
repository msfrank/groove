#include <iostream>
#include <string>

#include <groove_units/reference_system_manager.h>

#include "antlr4-runtime.h"
#include "UnitsLexer.h"
#include "UnitsParser.h"

#include <groove_units/internal/units_listener.h>

groove_units::ReferenceSystemManager::ReferenceSystemManager()
{
}

bool
groove_units::ReferenceSystemManager::containsReferenceSystem(std::string_view name) const
{
    return m_systems.contains(name);
}

groove_units::UnitReferenceSystem
groove_units::ReferenceSystemManager::getReferenceSystem(std::string_view name) const
{
    if (m_systems.contains(name))
        return m_systems.at(name);
    return {};
}

void
groove_units::ReferenceSystemManager::setReferenceSystem(std::string_view name, const UnitReferenceSystem &rs)
{
    m_systems[name] = rs;
}

void
groove_units::ReferenceSystemManager::removeReferenceSystem(std::string_view name)
{
    m_systems.erase(name);
}

std::vector<std::string>
groove_units::ReferenceSystemManager::listReferenceSystems() const
{
    std::vector<std::string> referenceSystems;
    for (const auto &name : m_systems) {
        referenceSystems.push_back(name.first);
    }
    return referenceSystems;
}

tempo_utils::Result<groove_units::UnitDimension>
groove_units::ReferenceSystemManager::parse(std::string_view utf8) const
{
    if (utf8.empty())
        return UnitDimension();

    antlr4::ANTLRInputStream input(utf8.data(), (size_t) utf8.size());
    UnitsLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    UnitsParser parser(&tokens);

    internal::UnitsListener listener(this);

    antlr4::tree::ParseTree *tree = parser.rsUnits();
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    auto rs = listener.getReferenceSystem();
    return rs.parse(listener.getUnits(), listener.getScaleBase(), listener.getScaleExponent());
}
