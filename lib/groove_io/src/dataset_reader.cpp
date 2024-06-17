
#include <arrow/util/key_value_metadata.h>

#include <groove_io/dataset_reader.h>
#include <groove_io/groove_index.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/memory_mapped_bytes.h>

groove_io::DatasetReader::DatasetReader(
    const std::filesystem::path &datasetPath,
    tu_uint8 version,
    tu_uint8 flags,
    GrooveIndex index,
    groove_model::GrooveSchema schema,
    tempo_utils::Slice content)
    : m_datasetPath(datasetPath),
      m_version(version),
      m_flags(flags),
      m_index(index),
      m_schema(schema),
      m_content(content)
{
    TU_ASSERT (!m_datasetPath.empty());
    TU_ASSERT (m_index.isValid());
    TU_ASSERT (m_schema.isValid());
}

bool
groove_io::DatasetReader::isValid() const
{
    return m_index.isValid();
}

std::filesystem::path
groove_io::DatasetReader::getDatasetPath() const
{
    return m_datasetPath;
}

tu_uint8
groove_io::DatasetReader::getVersion() const
{
    return m_version;
}

tu_uint8
groove_io::DatasetReader::getFlags() const
{
    return m_flags;
}

groove_model::GrooveSchema
groove_io::DatasetReader::getSchema() const
{
    return m_schema;
}

bool
groove_io::DatasetReader::isEmpty()
{
    if (!isValid())
        return false;
    return m_index.getIndex().numVectors() == 0;
}

tempo_utils::Result<groove_model::PageId>
groove_io::DatasetReader::getPageIdBefore(
    const groove_model::PageId &pageId,
    bool exclusive)
{
    if (!isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant, "reader is not valid");

    auto index = m_index.getIndex();
    auto vector = index.findVectorBefore(pageId);
    if (!vector.isValid() && !exclusive) {
        vector = index.findVector(pageId);
    }
    if (!vector.isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant);

    return vector.getPageId();
}

tempo_utils::Result<groove_model::PageId>
groove_io::DatasetReader::getPageIdAfter(
    const groove_model::PageId &pageId,
    bool exclusive)
{
    if (!isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant, "reader is not valid");

    auto index = m_index.getIndex();
    auto vector = index.findVectorAfter(pageId);
    if (!vector.isValid() && !exclusive) {
        vector = index.findVector(pageId);
    }
    if (!vector.isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant);

    return vector.getPageId();
}

tempo_utils::Result<std::shared_ptr<arrow::Buffer>>
groove_io::DatasetReader::getPageData(
    const groove_model::PageId &pageId)
{
    if (!isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant, "reader is not valid");

    auto index = m_index.getIndex();
    auto vector = index.findVector(pageId);
    if (!vector.isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant);

    auto frame = index.getFrame(vector.getFrameIndex());
    if (!frame.isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant);
    if (frame.getFrameOffset() + frame.getFrameSize() > m_content.getSize())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant, "invalid page");

    //auto *contentBase = m_content.
    auto frameBytes = m_content.slice(frame.getFrameOffset(), frame.getFrameSize());
    auto pageData = std::make_shared<DatasetPage>(frameBytes);

    return std::static_pointer_cast<arrow::Buffer>(pageData);
}

tempo_utils::Status
groove_io::DatasetReader::pageExists(const groove_model::PageId &pageId)
{
    if (!isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant, "reader is not valid");

    auto index = m_index.getIndex();
    auto vector = index.findVector(pageId);
    if (!vector.isValid())
        return groove_model::ModelStatus::forCondition(
            groove_model::ModelCondition::kModelInvariant);

    return groove_model::ModelStatus::ok();
}

tempo_utils::Result<std::shared_ptr<groove_io::DatasetReader>>
groove_io::DatasetReader::create(const std::filesystem::path &datasetPath)
{
    std::shared_ptr<tempo_utils::MemoryMappedBytes> fileMapping;
    TU_ASSIGN_OR_RETURN (fileMapping, tempo_utils::MemoryMappedBytes::open(datasetPath));

    auto *mmapData = fileMapping->getData();
    auto mmapSize = fileMapping->getSize();

    // verify the package header
    if (mmapSize < 10)
        return IOStatus::forCondition(
            IOCondition::kIOInvariant, "invalid header size");
    if (strncmp((const char *) mmapData, gii1::IndexIdentifier(), 4) != 0)
        return IOStatus::forCondition(
            IOCondition::kIOInvariant, "invalid dataset identifier");
    mmapData += 4;

    auto version = tempo_utils::read_u8_and_advance(mmapData);
    auto flags = tempo_utils::read_u8_and_advance(mmapData);
    auto indexSize = tempo_utils::read_u32_and_advance(mmapData);
    mmapSize -= 10;

    if (mmapSize < indexSize)
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset index");
    std::span indexSpan((const uint8_t *) mmapData, indexSize);
    if (!GrooveIndex::verify(indexSpan, /* noIdentifier= */ true))
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset index");

    // allocate the index
    tempo_utils::Slice indexSlice(fileMapping, 10, indexSize);
    GrooveIndex index(indexSlice.toImmutableBytes());
    if (!index.isValid())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset index");
    mmapData += indexSize;
    mmapSize -= indexSize;

    // verify the schema
    if (mmapSize < 4)
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset schema");
    auto schemaSize = tempo_utils::read_u32_and_advance(mmapData);
    mmapSize -= 4;
    if (mmapSize < schemaSize)
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset schema");
    std::span schemaSpan((const uint8_t *) mmapData, schemaSize);
    if (!groove_model::GrooveSchema::verify(schemaSpan, /* noIdentifier= */ true))
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset schema");

    // allocate the schema
    tempo_utils::Slice schemaSlice(fileMapping, 10 + 4 + indexSize, schemaSize);
    groove_model::GrooveSchema schema(schemaSlice.toImmutableBytes());
    if (!schema.isValid())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid dataset schema");
    mmapData += schemaSize;
    mmapSize -= schemaSize;
    tempo_utils::Slice content(fileMapping, 10 + 4 + indexSize + schemaSize);

    // finally create the reader
    auto *reader = new DatasetReader(datasetPath, version, flags, index, schema, content);
    return std::shared_ptr<DatasetReader>(reader);
}

groove_io::DatasetPage::DatasetPage(tempo_utils::Slice bytes)
    : arrow::Buffer(bytes.getData(), bytes.getSize()),
      m_bytes(bytes)
{
}