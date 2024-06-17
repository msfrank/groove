
#include <groove/schema_builder.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_state.h>

tempo_utils::Result<groove_model::GrooveSchema>
make_schema(const absl::flat_hash_map<std::string,ModelConfig> &modelConfigs)
{
    groove_model::SchemaState state;

    for (const auto &modelEntry : modelConfigs) {
        const auto &modelId = modelEntry.first;

        const auto &modelConfig = modelEntry.second;

        // set model key type
        groove_model::ModelKeyType modelKeyType = groove_model::ModelKeyType::Invalid;
        switch (modelConfig.keyType) {
            case groove_data::DataKeyType::KEY_CATEGORY:
                modelKeyType = groove_model::ModelKeyType::Category;
                break;
            case groove_data::DataKeyType::KEY_DOUBLE:
                modelKeyType = groove_model::ModelKeyType::Double;
                break;
            case groove_data::DataKeyType::KEY_INT64:
                modelKeyType = groove_model::ModelKeyType::Int64;
                break;
            default:
                return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                    "invalid key type for model");
        }

        // set model collation mode
        groove_model::ModelKeyCollation modelKeyCollation = groove_model::ModelKeyCollation::Invalid;
        switch (modelConfig.collation) {
            case groove_data::CollationMode::COLLATION_INDEXED:
                modelKeyCollation = groove_model::ModelKeyCollation::Indexed;
                break;
            case groove_data::CollationMode::COLLATION_SORTED:
                modelKeyCollation = groove_model::ModelKeyCollation::Sorted;
                break;
            default:
                return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                    "invalid collation mode for model");
        }

        groove_model::SchemaModel *model;
        TU_ASSIGN_OR_RETURN (model, state.putModel(modelId, modelKeyType, modelKeyCollation));

        // append each column
        for (const auto &columnEntry : modelConfig.columns) {
            const auto &columnId = columnEntry.first;

            const auto &columnConfig = columnEntry.second;

            groove_model::ColumnValueType columnValueType;
            switch (columnConfig.valueType) {
                case groove_data::DataValueType::VALUE_TYPE_DOUBLE:
                    columnValueType = groove_model::ColumnValueType::Double;
                    break;
                case groove_data::DataValueType::VALUE_TYPE_INT64:
                    columnValueType = groove_model::ColumnValueType::Int64;
                    break;
                case groove_data::DataValueType::VALUE_TYPE_STRING:
                    columnValueType = groove_model::ColumnValueType::String;
                    break;
                default:
                    return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                        "invalid value type for column");
            }

            groove_model::ColumnValueFidelity columnValueFidelity;
            if (columnConfig.nullsAllowed) {
                if (!columnConfig.fidelityField.empty()) {
                    columnValueFidelity = groove_model::ColumnValueFidelity::AnyFidelityAllowed;
                } else {
                    columnValueFidelity = groove_model::ColumnValueFidelity::OnlyValidOrEmpty;
                }
            } else {
                columnValueFidelity = groove_model::ColumnValueFidelity::OnlyValidValue;
            }

            groove_model::SchemaColumn *column;
            TU_ASSIGN_OR_RETURN (column, state.appendColumn(columnId, columnValueType, columnValueFidelity));
            TU_RETURN_IF_NOT_OK (model->appendColumn(column));
        }
    }

    return state.toSchema();
}
