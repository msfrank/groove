#ifndef GROOVE_UNITS_UNICODE_UTILS_H
#define GROOVE_UNITS_UNICODE_UTILS_H

#include <string>

#include <absl/container/flat_hash_map.h>

namespace groove_units {

    std::string pretty_print_exponent(int exp);

    std::string pretty_print_units(const absl::flat_hash_map<std::string, int> &units);
}

#endif // GROOVE_UNITS_UNICODE_UTILS_H
