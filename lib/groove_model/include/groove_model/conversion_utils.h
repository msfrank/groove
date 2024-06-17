#ifndef GROOVE_MODEL_CONVERSION_UTILS_H
#define GROOVE_MODEL_CONVERSION_UTILS_H

#include <string>

#include <groove_data/data_types.h>
#include <tempo_utils/integer_types.h>
#include <tempo_utils/option_template.h>

namespace groove_model {

    std::string int64_to_bytes(tu_int64 src);

    std::string double_to_bytes(double src);

    std::string key_to_bytes(Option<tu_int64> key);

    std::string key_to_bytes(Option<double> key);

    std::string key_to_bytes(const Option<groove_data::Category> key);

};

#endif // GROOVE_MODEL_CONVERSION_UTILS_H