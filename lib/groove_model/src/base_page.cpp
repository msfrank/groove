
#include <arrow/io/memory.h>
#include <arrow/ipc/message.h>
#include <arrow/ipc/reader.h>
#include <arrow/ipc/writer.h>

#include <groove_model/base_page.h>

//groove_model::BasePage::BasePage(PageType pageType)
//    : m_type(pageType)
//{
//    TU_ASSERT (m_type != PageType::PAGE_TYPE_UNKNOWN);
//}
//
//groove_model::PageType
//groove_model::BasePage::getPageType() const
//{
//    return m_type;
//}

groove_model::BasePage::BasePage(PageId pageId, std::shared_ptr<const std::string> columnId)
    : m_pageId(pageId),
      m_columnId(columnId)
{
}

groove_model::PageId
groove_model::BasePage::getPageId() const
{
    return m_pageId;
}

std::shared_ptr<const std::string>
groove_model::BasePage::getColumnId() const
{
    return m_columnId;
}

//arrow::Result<std::shared_ptr<arrow::Table>>
//groove_model::BasePage::fromBuffer(std::shared_ptr<arrow::Buffer> buffer)
//{
//    TU_ASSERT (buffer != nullptr);
//
//    if (buffer->size() == 0)
//        return nullptr;
//
//    arrow::io::BufferReader bufferReader(buffer);
//    auto messageReader = arrow::ipc::MessageReader::Open(&bufferReader);
//    auto openStreamReaderResult = arrow::ipc::RecordBatchStreamReader::Open(std::move(messageReader));
//    if (!openStreamReaderResult.ok())
//        return nullptr;
//    auto streamReader = *openStreamReaderResult;
//    return streamReader->ToTable();
//}
//
//arrow::Result<std::shared_ptr<arrow::Table>>
//groove_model::BasePage::fromBytes(std::shared_ptr<const std::string> bytes)
//{
//    if (bytes->empty())
//        return nullptr;
//
//    // the arrow table needs a buffer to hold the data for the lifetime of the table
//    // TODO: we could reduce copies when loading from rocksdb by using PinnableSlice
//    arrow::BufferBuilder builder;
//    arrow::Status status;
//    status = builder.Resize(bytes->size());
//    if (!status.ok())
//        return arrow::Result<std::shared_ptr<arrow::Table>>(status);
//    status = builder.Append(bytes->data(), bytes->size());
//    if (!status.ok())
//        return arrow::Result<std::shared_ptr<arrow::Table>>(status);
//    auto finishBufferResult = builder.Finish();
//    if (!finishBufferResult.ok())
//        return arrow::Result<std::shared_ptr<arrow::Table>>(finishBufferResult.status());
//    std::shared_ptr<arrow::Buffer> buffer = *finishBufferResult;
//
//    arrow::io::BufferReader bufferReader(buffer);
//    auto messageReader = arrow::ipc::MessageReader::Open(&bufferReader);
//    auto openStreamReaderResult = arrow::ipc::RecordBatchStreamReader::Open(std::move(messageReader));
//    if (!openStreamReaderResult.ok())
//        return nullptr;
//    auto streamReader = *openStreamReaderResult;
//    return streamReader->ToTable();
//}
