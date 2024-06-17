
#include <groove_model/shared_string_buffer.h>

groove_model::SharedStringBuffer::SharedStringBuffer(std::shared_ptr<const std::string> bytes)
    : arrow::Buffer(std::string_view(bytes->data(), bytes->size())),
      m_bytes(bytes)
{
    TU_ASSERT (m_bytes != nullptr);
}

std::shared_ptr<groove_model::SharedStringBuffer>
groove_model::SharedStringBuffer::create(std::string_view &bytes)
{
    return std::make_shared<SharedStringBuffer>(std::make_shared<const std::string>(bytes));
}

std::shared_ptr<groove_model::SharedStringBuffer>
groove_model::SharedStringBuffer::create(const std::string &bytes)
{
    return std::make_shared<SharedStringBuffer>(std::make_shared<const std::string>(bytes));
}

std::shared_ptr<groove_model::SharedStringBuffer>
groove_model::SharedStringBuffer::create(std::string &&bytes)
{
    return std::make_shared<SharedStringBuffer>(std::make_shared<const std::string>(std::move(bytes)));
}