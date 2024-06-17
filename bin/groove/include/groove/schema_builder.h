#ifndef GROOVE_SCHEMA_BUILDER_H
#define GROOVE_SCHEMA_BUILDER_H

#include <groove_model/groove_schema.h>
#include <tempo_command/command_result.h>

#include "model_config_parser.h"

tempo_utils::Result<groove_model::GrooveSchema>
make_schema(const absl::flat_hash_map<std::string,ModelConfig> &modelConfigs);

#endif // GROOVE_SCHEMA_BUILDER_H