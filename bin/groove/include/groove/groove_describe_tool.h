#ifndef GROOVE_DESCRIBE_TOOL_H
#define GROOVE_DESCRIBE_TOOL_H

#include <tempo_command/command_config.h>
#include <tempo_command/command_result.h>
#include <tempo_command/command_tokenizer.h>

tempo_utils::Status
groove_describe_tool(
    const std::filesystem::path &workspaceRoot,
    const std::filesystem::path &distributionRoot,
    tempo_command::TokenVector &tokens);

#endif // GROOVE_DESCRIBE_TOOL_H
