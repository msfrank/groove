
#include <arrow/array.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/message.h>
#include <arrow/ipc/reader.h>
#include <arrow/ipc/writer.h>
#include <arrow/table.h>

#include <groove_data/category_double_vector.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/double_string_vector.h>
#include <groove_data/int64_double_vector.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/int64_string_vector.h>
#include <groove_io/dataset_writer.h>
#include <groove_io/index_state.h>
#include <tempo_utils/file_appender.h>

groove_io::DatasetWriter::DatasetWriter(const tempo_utils::Url &datasetUrl, const groove_model::GrooveSchema &schema)
    : DatasetWriter(datasetUrl, schema, {})
{
}

groove_io::DatasetWriter::DatasetWriter(
    const tempo_utils::Url &datasetUrl,
    const groove_model::GrooveSchema &schema,
    const DatasetWriterOptions &options)
    : m_datasetUrl(datasetUrl),
      m_schema(schema),
      m_options(options)
{
}

template <typename VectorType, typename KeyType>
static tempo_utils::Result<std::pair<groove_model::PageId,groove_model::PageId>>
make_page_id_pair_from_vector(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    std::shared_ptr<groove_data::BaseVector> vector)
{
    auto vec = std::static_pointer_cast<VectorType>(vector);
    Option<KeyType> smallestKey(vec->getSmallest().getValue().key);
    Option<KeyType> largestKey(vec->getLargest().getValue().key);
    return std::pair<groove_model::PageId,groove_model::PageId>(
        groove_model::PageId::create(datasetUrl, modelId, columnId, valueType, collation, smallestKey),
        groove_model::PageId::create(datasetUrl, modelId, columnId, valueType, collation, largestKey));
}

static tempo_utils::Result<std::pair<groove_model::PageId,groove_model::PageId>>
make_page_id(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataKeyType keyType,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    std::shared_ptr<groove_data::BaseVector> vector)
{
    TU_ASSERT (vector != nullptr);

    if (vector->isEmpty())
        return std::pair<groove_model::PageId,groove_model::PageId>();

    switch (vector->getVectorType()) {
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE: {
            return make_page_id_pair_from_vector<groove_data::CategoryDoubleVector,groove_data::Category>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_INT64: {
            return make_page_id_pair_from_vector<groove_data::CategoryInt64Vector,groove_data::Category>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_CATEGORY_STRING: {
            return make_page_id_pair_from_vector<groove_data::CategoryStringVector,groove_data::Category>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE: {
            return make_page_id_pair_from_vector<groove_data::DoubleDoubleVector,double>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_INT64: {
            return make_page_id_pair_from_vector<groove_data::DoubleInt64Vector,double>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_DOUBLE_STRING: {
            return make_page_id_pair_from_vector<groove_data::DoubleStringVector,double>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_DOUBLE: {
            return make_page_id_pair_from_vector<groove_data::Int64DoubleVector,tu_int64>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_INT64: {
            return make_page_id_pair_from_vector<groove_data::Int64Int64Vector,tu_int64>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        case groove_data::DataVectorType::VECTOR_TYPE_INT64_STRING: {
            return make_page_id_pair_from_vector<groove_data::Int64StringVector,tu_int64>
                (datasetUrl, modelId, columnId, valueType, collation, vector);
        }
        default:
            return groove_io::IOStatus::forCondition(
                groove_io::IOCondition::kIOInvariant, "invalid vector type");
    }
}

static std::shared_ptr<const arrow::Buffer> serialize_table(std::shared_ptr<const arrow::Table> table)
{
    auto createBufResult = arrow::io::BufferOutputStream::Create();
    if (!createBufResult.ok())
        return {};
    auto stream = *createBufResult;

    auto schema = table->schema();
    auto makeWriterResult = arrow::ipc::MakeStreamWriter(stream, schema);
    if (!makeWriterResult.ok())
        return {};
    auto writer = *makeWriterResult;

    arrow::Status status;
    status = writer->WriteTable(*table);
    if (!status.ok())
        return {};
    status = writer->Close();
    if (!status.ok())
        return {};
    auto finishStreamResult = stream->Finish();
    if (!finishStreamResult.ok())
        return {};
    auto buffer = *finishStreamResult;
    return buffer;
}

tempo_utils::Status
groove_io::DatasetWriter::putVector(
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseVector> vector,
    tu_uint32 frameIndex)
{
    const auto model = m_schema.getSchema().findModel(modelId);
    if (!model.isValid())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "missing model with specified id");
    if (model.numColumns() == 0)
        return IOStatus::forCondition(IOCondition::kIOInvariant, "invalid model has no columns");

    groove_data::DataKeyType keyType = groove_data::DataKeyType::INVALID;
    switch (model.getKeyType()) {
        case groove_model::ModelKeyType::Category:
            keyType = groove_data::DataKeyType::KEY_CATEGORY;
            break;
        case groove_model::ModelKeyType::Double:
            keyType = groove_data::DataKeyType::KEY_DOUBLE;
            break;
        case groove_model::ModelKeyType::Int64:
            keyType = groove_data::DataKeyType::KEY_INT64;
            break;
        default:
            break;
    }
    if (keyType != vector->getKeyType())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "model key type does not match vector");

    //
    groove_data::CollationMode collation = groove_data::CollationMode::INVALID;
    switch (model.getKeyCollation()) {
        case groove_model::ModelKeyCollation::Indexed:
            collation = groove_data::CollationMode::COLLATION_INDEXED;
            break;
        case groove_model::ModelKeyCollation::Sorted:
            collation = groove_data::CollationMode::COLLATION_SORTED;
            break;
        default:
            break;
    }

    //
    auto columnId = vector->getColumnId();
    auto column = model.findColumn(columnId);
    if (!column.isValid())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "missing column with specified id");

    //
    groove_data::DataValueType valueType = groove_data::DataValueType::INVALID;
    switch (column.getValueType()) {
        case groove_model::ColumnValueType::Double:
            valueType = groove_data::DataValueType::VALUE_TYPE_DOUBLE;
            break;
        case groove_model::ColumnValueType::Int64:
            valueType = groove_data::DataValueType::VALUE_TYPE_INT64;
            break;
        case groove_model::ColumnValueType::String:
            valueType = groove_data::DataValueType::VALUE_TYPE_STRING;
            break;
        default:
            break;
    }
    if (valueType != vector->getValueType())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "column value type does not match vector");

    // generate the page id for the vector
    auto makePageIdResult = make_page_id(m_datasetUrl,
        std::make_shared<const std::string>(modelId),
        std::make_shared<const std::string>(columnId),
        keyType, valueType, collation, vector);
    if (makePageIdResult.isStatus())
        return makePageIdResult.getStatus();
    auto pageIdPair = makePageIdResult.getResult();

    // check intervals to ensure that vector doesn't overlap with existing data
    if (!m_pageIdToOffset.empty()) {
        auto iterator = m_pageIdToOffset.lower_bound(pageIdPair.first);

        if (iterator != m_pageIdToOffset.cend()) {

            // check if page exists with the specified page id
            if (iterator->first == pageIdPair.first)
                return IOStatus::forCondition(IOCondition::kIOInvariant, "page id already exists");

            // check if the page prior to iterator overlaps with the smallest key
            if (iterator != m_pageIdToOffset.cbegin()) {
                iterator--;
                if (pageIdPair.second < iterator->second.second)
                    return IOStatus::forCondition(IOCondition::kIOInvariant, "prior page id overlaps");
                iterator++;
            }

            // check if the page at iterator overlaps with the largest key
            if (iterator->first < pageIdPair.second)
                return IOStatus::forCondition(IOCondition::kIOInvariant, "subsequent page id overlaps");

        } else {
            iterator = m_pageIdToOffset.end();
            iterator--;
            if (pageIdPair.second < iterator->second.second)
                return IOStatus::forCondition(IOCondition::kIOInvariant, "prior page id overlaps");
        }
    }

    // check if the page at iterator overlaps with the largest key

    VectorPriv vectorPriv;
    vectorPriv.pageId = pageIdPair.first;
    vectorPriv.frameIndex = frameIndex;
    vectorPriv.valOffset = vector->getValFieldIndex();
    vectorPriv.fidOffset = vector->getFidFieldIndex() < 0? kInvalidOffsetU32 : vector->getFidFieldIndex();
    m_vectors.push_back(vectorPriv);
    m_pageIdToOffset[pageIdPair.first] = {frameIndex, pageIdPair.second};

    return IOStatus::ok();
}

tempo_utils::Status
groove_io::DatasetWriter::putVector(
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseVector> vector)
{
    auto underlyingSchema = vector->getSchema();
    auto underlyingTable = vector->getTable();

    std::vector<std::shared_ptr<arrow::Field>> fields;
    fields.push_back(underlyingSchema->field(vector->getKeyFieldIndex()));
    fields.push_back(underlyingSchema->field(vector->getValFieldIndex()));
    if (vector->getFidFieldIndex() >= 0) {
        fields.push_back(underlyingSchema->field(vector->getFidFieldIndex()));
    }
    auto schema = arrow::schema(fields);

    if (m_options.stripMetadata) {
        schema = schema->RemoveMetadata();
    }

    std::vector<std::shared_ptr<arrow::ChunkedArray>> columns;
    columns.push_back(underlyingTable->column(vector->getKeyFieldIndex()));
    columns.push_back(underlyingTable->column(vector->getValFieldIndex()));
    if (vector->getFidFieldIndex() >= 0) {
        columns.push_back(underlyingTable->column(vector->getFidFieldIndex()));
    }
    auto table = arrow::Table::Make(schema, columns);

    //
    auto framePriv = std::make_unique<FramePriv>();
    framePriv->keyOffset = vector->getKeyFieldIndex();
    framePriv->frameBytes = serialize_table(table);

    tu_uint32 frameIndex = m_frames.size();
    m_frames.push_back(std::move(framePriv));

    return putVector(modelId, vector, frameIndex);
}

tempo_utils::Status
groove_io::DatasetWriter::putFrame(
    const std::string &modelId,
    std::shared_ptr<groove_data::BaseFrame> frame)
{
    auto framePriv = std::make_unique<FramePriv>();
    framePriv->keyOffset = frame->getKeyFieldIndex();
    framePriv->frameBytes = serialize_table(frame->getUnderlyingTable());

    tu_uint32 frameIndex = m_frames.size();
    m_frames.push_back(std::move(framePriv));

    for (auto iterator = frame->vectorsBegin(); iterator != frame->vectorsEnd(); iterator++) {
        auto vector = iterator->second;
        auto status = putVector(modelId, vector, frameIndex);
    }

    return IOStatus::ok();
}

tempo_utils::Status
groove_io::DatasetWriter::writeDataset(const std::filesystem::path &datasetPath)
{
    IndexState state;

    for (const auto &vector : m_vectors) {
        FrameAddress frameAddress(vector.frameIndex);
        if (vector.fidOffset != kInvalidOffsetU32) {
            state.appendVector(vector.pageId, frameAddress, vector.valOffset, vector.fidOffset);
        } else {
            state.appendVector(vector.pageId, frameAddress, vector.valOffset);
        }
    }

    uint32_t currOffset = 0;
    for (const auto &frame : m_frames) {
        auto frameSize = frame->frameBytes->size();
        auto frameOffset = currOffset;
        state.appendFrame(frame->keyOffset, frameOffset, frameSize);
        currOffset += frameSize;
    }

    // serialize the index
    auto toIndexResult = state.toIndex();
    if (toIndexResult.isStatus())
        return toIndexResult.getStatus();
    auto index = toIndexResult.getResult();

    // create appender for the dataset file
    tempo_utils::FileAppender appender(datasetPath, tempo_utils::FileAppenderMode::CREATE_ONLY);
    auto status = appender.getStatus();
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to open dataset path for writing");

    // write the index file identifier
    status = appender.appendBytes({gii1::IndexIdentifier(), strlen(gii1::IndexIdentifier())});
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset file identifier");

    // write the prologue
    status = appender.appendU8(1);                                                  // version: u8
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset file version");
    status = appender.appendU8(0);                                                  // flags: u8
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset file flags");

    // write the index
    auto indexBytes = index.bytesView();
    status = appender.appendU32(indexBytes.size());                                 // indexSize: u32
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset index size");
    status = appender.appendBytes({indexBytes.data(), indexBytes.size()});          // index: bytes
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset index");

    // write the schema
    auto schemaBytes = m_schema.bytesView();
    status = appender.appendU32(schemaBytes.size());                                // schemaSize: u32
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset schema size");
    status = appender.appendBytes({schemaBytes.data(), schemaBytes.size()});        // schema: bytes
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset schema");

    // write each frame contents
    for (tu_uint32 i = 0; i < m_frames.size(); i++) {
        const auto &frame = m_frames.at(i);
        auto frameBytes = frame->frameBytes->data();
        auto frameSize = frame->frameBytes->size();
        if (frameSize < 0 || frameSize > std::numeric_limits<tu_uint32>::max())
            return IOStatus::forCondition(IOCondition::kIOInvariant, "dataset frame is too large");
        status = appender.appendBytes({frameBytes, static_cast<tu_uint32>(frameSize)});        // frame: bytes
        if (status.notOk())
            return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset frame");
    }

    status = appender.finish();
    if (status.notOk())
        return IOStatus::forCondition(IOCondition::kIOInvariant, "failed to write dataset");

    return IOStatus::ok();
}