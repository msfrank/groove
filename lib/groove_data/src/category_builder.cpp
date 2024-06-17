
#include <groove_data/category_builder.h>
#include <tempo_utils/log_stream.h>

groove_data::CategoryBuilder::CategoryBuilder()
{
    m_str = std::make_shared<arrow::StringBuilder>();
    m_cat = std::make_unique<arrow::ListBuilder>(arrow::system_memory_pool(), m_str);
}

arrow::Status
groove_data::CategoryBuilder::Append(const Category category)
{
    TU_ASSERT (!category.isEmpty());

    auto status = m_cat->Append();
    if (!status.ok())
        return status;
    std::vector<const char *> offsets;
    for (auto iterator = category.cbegin(); iterator != category.cend(); iterator++) {
        auto part = *iterator;
        offsets.push_back(part->c_str());
    }
    return m_str->AppendValues(offsets.data(), offsets.size());
}

arrow::Result<std::shared_ptr<arrow::Array>>
groove_data::CategoryBuilder::Finish()
{
    return m_cat->Finish();
}

std::shared_ptr<arrow::DataType>
groove_data::CategoryBuilder::makeDatatype()
{
    return arrow::list(arrow::utf8());
}
