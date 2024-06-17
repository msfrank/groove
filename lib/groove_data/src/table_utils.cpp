
#include <arrow/buffer.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/message.h>
#include <arrow/ipc/reader.h>
#include <arrow/ipc/writer.h>
#include <arrow/buffer_builder.h>

#include <groove_data/table_utils.h>

tempo_utils::Result<std::shared_ptr<arrow::Table>>
groove_data::make_table(std::shared_ptr<arrow::Buffer> buffer)
{
    TU_ASSERT (buffer != nullptr);

    if (buffer->size() == 0)
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid buffer");

    arrow::io::BufferReader bufferReader(buffer);
    auto messageReader = arrow::ipc::MessageReader::Open(&bufferReader);
    auto openStreamReaderResult = arrow::ipc::RecordBatchStreamReader::Open(std::move(messageReader));
    if (!openStreamReaderResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to parse buffer");
    auto streamReader = *openStreamReaderResult;
    auto toTableResult = streamReader->ToTable();
    if (!toTableResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to make table");
    return *toTableResult;
}

tempo_utils::Result<std::shared_ptr<arrow::Table>>
groove_data::make_table(std::shared_ptr<const std::string> bytes)
{
    if (bytes->empty())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "invalid buffer");

    // the arrow table needs a buffer to hold the data for the lifetime of the table
    arrow::BufferBuilder builder;
    arrow::Status status;
    status = builder.Resize(bytes->size());
    if (!status.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to resize buffer");
    status = builder.Append(bytes->data(), bytes->size());
    if (!status.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to resize buffer");
    auto finishBufferResult = builder.Finish();
    if (!finishBufferResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to finalize buffer");
    std::shared_ptr<arrow::Buffer> buffer = *finishBufferResult;

    arrow::io::BufferReader bufferReader(buffer);
    auto messageReader = arrow::ipc::MessageReader::Open(&bufferReader);
    auto openStreamReaderResult = arrow::ipc::RecordBatchStreamReader::Open(std::move(messageReader));
    if (!openStreamReaderResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to parse buffer");
    auto streamReader = *openStreamReaderResult;
    auto toTableResult = streamReader->ToTable();
    if (!toTableResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to make table");
    return *toTableResult;
}

tempo_utils::Result<std::shared_ptr<const arrow::Buffer>>
groove_data::make_buffer(std::shared_ptr<const arrow::Table> table)
{
    auto schema = table->schema();

    auto createBufResult = arrow::io::BufferOutputStream::Create();
    if (!createBufResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to create output stream");
    auto stream = *createBufResult;

    auto makeWriterResult = arrow::ipc::MakeStreamWriter(stream, schema);
    if (!makeWriterResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to create stream writer");
    auto writer = *makeWriterResult;

    arrow::Status status;
    status = writer->WriteTable(*table);
    if (!status.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to write table");
    status = writer->Close();
    if (!status.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to write table");
    auto finishStreamResult = stream->Finish();
    if (!finishStreamResult.ok())
        return DataStatus::forCondition(DataCondition::kDataInvariant, "failed to write table");
    return std::static_pointer_cast<const arrow::Buffer>(*finishStreamResult);
}
