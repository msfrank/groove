#ifndef GROOVE_DATA_CONVERTER_H
#define GROOVE_DATA_CONVERTER_H

#include <arrow/table.h>

#include <groove_data/base_frame.h>
#include <tempo_command/command_result.h>

#include "model_config_parser.h"

typedef tempo_utils::Result<std::shared_ptr<groove_data::BaseFrame>>
(*ExtensionConverterFunc)(const ModelConfig &, const std::filesystem::path &);

struct ExtensionConverter {
    DataFileType type;
    const char *extension;
    ExtensionConverterFunc converter;
};

tempo_utils::Result<std::shared_ptr<groove_data::BaseFrame>>
convert_json_input(const ModelConfig &modelConfig, const std::filesystem::path &path);

#endif // GROOVE_DATA_CONVERTER_H