
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/double_frame.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::DoubleFrame::DoubleFrame(std::shared_ptr<arrow::Table> table, int keyFieldIndex)
    : BaseFrame(table),
      m_keyFieldIndex(keyFieldIndex)
{
    TU_ASSERT (m_keyFieldIndex >= 0);
}

groove_data::DataFrameType
groove_data::DoubleFrame::getFrameType() const
{
    return DataFrameType::FRAME_TYPE_DOUBLE;
}

groove_data::DataKeyType
groove_data::DoubleFrame::getKeyType() const
{
    return DataKeyType::KEY_DOUBLE;
}

int
groove_data::DoubleFrame::getKeyFieldIndex() const
{
    return m_keyFieldIndex;
}

Option<double>
groove_data::DoubleFrame::smallestKey() const
{
    auto table = getTable();
    if (table->num_rows() == 0)
        return {};
    auto keyArray = table->column(m_keyFieldIndex);
    double key;
    if (groove_data::get_double_datum(keyArray, 0, key))
        return Option<double>(key);
    return {};
}

Option<double>
groove_data::DoubleFrame::largestKey() const
{
    auto table = getTable();
    if (table->num_rows() == 0)
        return {};
    auto keyArray = table->column(m_keyFieldIndex);
    double key;
    if (groove_data::get_double_datum(keyArray, table->num_rows() - 1, key))
        return Option<double>(key);
    return {};
}

tempo_utils::Result<std::shared_ptr<groove_data::DoubleFrame>>
groove_data::DoubleFrame::create(
    std::shared_ptr<arrow::Table> table,
    int keyFieldIndex,
    const std::vector<std::pair<int,int>> &valueColumns)
{
    if (!table)
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid table");
    auto schema = table->schema();
    auto numFields = schema->num_fields();
    if (schema->num_fields() <= keyFieldIndex)
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid key index");
    auto keyField = table->schema()->field(keyFieldIndex);
    if (!keyField->type()->Equals(arrow::float64()))
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid key index");
    if (valueColumns.empty())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "no columns");

    auto frame = std::shared_ptr<DoubleFrame>(new DoubleFrame(table, keyFieldIndex));

    absl::flat_hash_set<std::string> columnIdsSeen;

    for (const auto &columnEntry : valueColumns) {
        const auto &valFieldIndex = columnEntry.first;
        const auto &fidFieldIndex = columnEntry.second;

        auto valField = schema->field(valFieldIndex);
        auto columnId = valField->name();

        if (columnId.empty())
            return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid column id");
        if (columnIdsSeen.contains(columnId))
            return DataStatus::forCondition(DataCondition::kDataInvariant, "column already exists");
        if (valFieldIndex < 0 || numFields <= valFieldIndex || valFieldIndex == keyFieldIndex)
            return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid val field index");
        if (fidFieldIndex >= 0) {
            if (numFields <= fidFieldIndex || fidFieldIndex == keyFieldIndex)
                return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid fid field index");
            auto fidField = schema->field(fidFieldIndex);
            if (fidField->type()->id() != arrow::Type::BOOL)
                return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid fid field index");
        }

        switch (valField->type()->id()) {
            case arrow::Type::DOUBLE: {
                auto vector = DoubleDoubleVector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
                frame->insertVector(columnId, vector);
                break;
            }
            case arrow::Type::INT64: {
                auto vector = DoubleInt64Vector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
                frame->insertVector(columnId, vector);
                break;
            }
            case arrow::Type::STRING: {
                auto vector = DoubleStringVector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
                frame->insertVector(columnId, vector);
                break;
            }
            default:
                return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid column type");
        }

        columnIdsSeen.insert(columnId);
    }

    return frame;
}
