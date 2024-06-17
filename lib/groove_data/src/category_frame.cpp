
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/category_frame.h>
#include <groove_data/category_double_vector.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::CategoryFrame::CategoryFrame(std::shared_ptr<arrow::Table> table, int keyFieldIndex)
    : BaseFrame(table),
      m_keyFieldIndex(keyFieldIndex)
{
    TU_ASSERT (m_keyFieldIndex >= 0);
}

groove_data::DataFrameType
groove_data::CategoryFrame::getFrameType() const
{
    return DataFrameType::FRAME_TYPE_CATEGORY;
}

groove_data::DataKeyType
groove_data::CategoryFrame::getKeyType() const
{
    return DataKeyType::KEY_CATEGORY;
}

int
groove_data::CategoryFrame::getKeyFieldIndex() const
{
    return m_keyFieldIndex;
}

Option<groove_data::Category>
groove_data::CategoryFrame::smallestKey() const
{
    auto table = getTable();
    if (table->num_rows() == 0)
        return {};
    auto keyArray = table->column(m_keyFieldIndex);
    Category key;
    if (groove_data::get_category_datum(keyArray, 0, key))
        return Option<Category>(key);
    return {};
}

Option<groove_data::Category>
groove_data::CategoryFrame::largestKey() const
{
    auto table = getTable();
    if (table->num_rows() == 0)
        return {};
    auto keyArray = table->column(m_keyFieldIndex);
    Category key;
    if (groove_data::get_category_datum(keyArray, table->num_rows() - 1, key))
        return Option<Category>(key);
    return {};
}

tempo_utils::Result<std::shared_ptr<groove_data::CategoryFrame>>
groove_data::CategoryFrame::create(
    std::shared_ptr<arrow::Table> table,
    int keyFieldIndex,
    const std::vector<std::pair<int,int>> &valueColumns)
{
    if (!table)
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid table");
    auto schema = table->schema();
    auto numFields = schema->num_fields();
    if (schema->num_fields() <= keyFieldIndex)
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid key field");
    auto keyField = table->schema()->field(keyFieldIndex);
    if (!keyField->type()->Equals(arrow::list(arrow::utf8())))
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid key field");
    if (valueColumns.empty())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "no columns");

    auto frame = std::shared_ptr<CategoryFrame>(new CategoryFrame(table, keyFieldIndex));

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
                auto vector = CategoryDoubleVector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
                frame->insertVector(columnId, vector);
                break;
            }
            case arrow::Type::INT64: {
                auto vector = CategoryInt64Vector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
                frame->insertVector(columnId, vector);
                break;
            }
            case arrow::Type::STRING: {
                auto vector = CategoryStringVector::create(table, keyFieldIndex, valFieldIndex, fidFieldIndex);
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