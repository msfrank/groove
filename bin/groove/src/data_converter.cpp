
#include <arrow/csv/reader.h>
#include <arrow/io/file.h>
#include <arrow/json/reader.h>

#include <groove/data_converter.h>
#include <groove_data/base_frame.h>
#include <groove_data/category_frame.h>
#include <groove_data/int64_frame.h>
#include <groove_data/double_frame.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>

tempo_utils::Result<std::shared_ptr<groove_data::BaseFrame>>
convert_json_input(const ModelConfig &modelConfig, const std::filesystem::path &path)
{
    auto readOptions = arrow::json::ReadOptions::Defaults();
    auto parseOptions = arrow::json::ParseOptions::Defaults();

    // if convertParameters is a map, then parse parameters
    if (modelConfig.convertParameters.getNodeType() == tempo_config::ConfigNodeType::kMap) {
        auto parametersMap = modelConfig.convertParameters.toMap();
        tempo_config::BooleanParser newlinesInValuesParser(false);
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(parseOptions.newlines_in_values, newlinesInValuesParser,
            parametersMap, "newlinesInValues"));
    }

    arrow::MemoryPool* pool = arrow::default_memory_pool();

    // open the input file
    auto openFileResult = arrow::io::ReadableFile::Open(path.string(), pool);
    if (!openFileResult.ok())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to open {}: {}", path.string(), openFileResult.status().ToString());

    // make the json reader
    auto makeReaderResult = arrow::json::TableReader::Make(pool, *openFileResult, readOptions, parseOptions);
    if (!makeReaderResult.ok())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to open {}: {}", path.string(), makeReaderResult.status().ToString());
    auto reader = *makeReaderResult;

    // Read input json and convert to an arrow table
    auto readJsonResult = reader->Read();
    if (!readJsonResult.ok())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to parse {}: {}", path.string(), readJsonResult.status().ToString());
    auto table = *readJsonResult;

    // map each schema field name to its field index
    auto schema = table->schema();
    absl::flat_hash_map<std::string,int> columnIdToIndexMap;
    for (int i = 0; i < schema->num_fields(); i++) {
        auto field = schema->field(i);
        auto columnId = field->name();
        if (columnIdToIndexMap.contains(columnId))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "schema contains duplicate column ids");
        columnIdToIndexMap[columnId] = i;
    }

    if (!columnIdToIndexMap.contains(modelConfig.keyField))
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "missing key field in schema");
    auto keyFieldIndex = columnIdToIndexMap.at(modelConfig.keyField);

    std::vector<std::pair<int,int>> valueColumns;
    for (const auto &columnEntry : modelConfig.columns) {
        const auto &columnId = columnEntry.first;
        const auto &columnConfig = columnEntry.second;

        if (!columnIdToIndexMap.contains(columnId))
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "missing value field in schema");
        auto valFieldIndex = columnIdToIndexMap.at(columnId);

        int fidFieldIndex = -1;
        if (!columnConfig.fidelityField.empty()) {
            const auto &fidelityField = columnConfig.fidelityField;
            if (!columnIdToIndexMap.contains(fidelityField))
                return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                    "missing fidelity field in schema");
            fidFieldIndex = columnIdToIndexMap.at(fidelityField);
        }

        valueColumns.push_back(std::pair<int,int>(valFieldIndex, fidFieldIndex));
    }

    //
    switch (modelConfig.keyType) {
        case groove_data::DataKeyType::KEY_CATEGORY: {
            auto createFrameResult = groove_data::CategoryFrame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus())
                return createFrameResult.getStatus();
            return std::static_pointer_cast<groove_data::BaseFrame>(createFrameResult.getResult());
        }
        case groove_data::DataKeyType::KEY_DOUBLE: {
            auto createFrameResult = groove_data::DoubleFrame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus())
                return createFrameResult.getStatus();
            return std::static_pointer_cast<groove_data::BaseFrame>(createFrameResult.getResult());
        }
        case groove_data::DataKeyType::KEY_INT64: {
            auto createFrameResult = groove_data::Int64Frame::create(table, keyFieldIndex, valueColumns);
            if (createFrameResult.isStatus())
                return createFrameResult.getStatus();
            return std::static_pointer_cast<groove_data::BaseFrame>(createFrameResult.getResult());
        }
        default:
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "invalid frame type");
    }
}
