
#include <absl/strings/str_join.h>

#include <groove_units/unicode_utils.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/unicode.h>

std::string
groove_units::pretty_print_exponent(int exp)
{
    std::string unicode;

    auto number = absl::StrCat(absl::AlphaNum(exp));
    for (const auto &chr : number) {
        switch (chr) {
            case '0':
                absl::StrAppend(&unicode, "⁰");
                break;
            case '1':
                absl::StrAppend(&unicode, "¹");
                break;
            case '2':
                absl::StrAppend(&unicode, "²");
                break;
            case '3':
                absl::StrAppend(&unicode, "³");
                break;
            case '4':
                absl::StrAppend(&unicode, "⁴");
                break;
            case '5':
                absl::StrAppend(&unicode, "⁵");
                break;
            case '6':
                absl::StrAppend(&unicode, "⁶");
                break;
            case '7':
                absl::StrAppend(&unicode, "⁷");
                break;
            case '8':
                absl::StrAppend(&unicode, "⁸");
                break;
            case '9':
                absl::StrAppend(&unicode, "⁹");
                break;
            default:
                TU_UNREACHABLE();
        }
    }

    return unicode;
}

std::string
groove_units::pretty_print_units(const absl::flat_hash_map<std::string,int> &units)
{
    if (units.empty())
        return {};

    std::vector<std::string> numerator;

    // generate the numerator.  this always orders quantities lexographically.
    for (const auto &unit: units) {
        auto exponent = unit.second;
        if (exponent <= 0)       // ignore denominator units and zero exponent units
            continue;
        if (exponent > 1) {
            // if the exponent is greater than 1, then display the exponent
            numerator.push_back(absl::StrCat(unit.first, pretty_print_exponent(exponent)));
        } else {
            // otherwise display the unit with no exponent
            numerator.push_back(unit.first);
        }
    }

    std::vector<std::string> denominator;

    // generate the denominator.  this always orders quantities lexographically.
    for (const auto &unit : units) {
        auto exponent = unit.second;
        if (exponent >= 0)      // ignore numerator units and zero exponent units
            continue;
        if (exponent > 1) {
            // if the exponent is greater than 1, then display the exponent
            denominator.push_back(absl::StrCat(unit.first, pretty_print_exponent(exponent)));
        } else {
            // otherwise display the unit with no exponent
            denominator.push_back(unit.first);
        }
    }

    if (denominator.empty())
        return absl::StrCat(absl::StrJoin(numerator, "·"));

    return absl::StrCat(
        absl::StrJoin(numerator, "·"),
        "/",
        absl::StrJoin(denominator, "·"));
}
