
#include <flatbuffers/idl.h>

#include <groove_io/generated/index_schema.h>
#include <groove_io/internal/index_reader.h>
#include <groove_io/io_types.h>
#include <tempo_utils/log_stream.h>

groove_io::internal::IndexReader::IndexReader(std::span<const tu_uint8> bytes)
    : m_bytes(bytes)
{
    m_index = gii1::GetIndex(m_bytes.data());
}

bool
groove_io::internal::IndexReader::isValid() const
{
    return m_index != nullptr;
}

gii1::IndexVersion
groove_io::internal::IndexReader::getABI() const
{
    if (m_index == nullptr)
        return gii1::IndexVersion::Unknown;
    return m_index->abi();
}

const gii1::VectorDescriptor *
groove_io::internal::IndexReader::getVector(uint32_t index) const
{
    if (m_index == nullptr)
        return nullptr;
    if (m_index->vectors() && index < m_index->vectors()->size())
        return m_index->vectors()->Get(index);
    return nullptr;
}

static bool comp_before(const gii1::VectorDescriptor* vector, std::string_view rhs)
{
    auto lhs = vector->page_id()->string_view();
    return lhs < rhs;
}

tu_uint32
groove_io::internal::IndexReader::findVector(std::string_view page_id) const
{
    if (m_index == nullptr)
        return kInvalidOffsetU32;
    if (m_index->vectors() == nullptr)
        return kInvalidOffsetU32;
    auto begin = m_index->vectors()->cbegin();
    auto end = m_index->vectors()->cend();

    // lower_bound returns element greater than or equal to page_id
    auto result = std::lower_bound(begin, end, page_id, comp_before);
    if (result == end)
        return kInvalidOffsetU32;

    // if page_id matches then return the element index, otherwise return invalid.
    if (result->page_id()->string_view() == page_id)
        return std::distance(m_index->vectors()->cbegin(), result);
    return kInvalidOffsetU32;
}

tu_uint32
groove_io::internal::IndexReader::findVectorBefore(const std::string_view page_id) const
{
    if (m_index == nullptr)
        return kInvalidOffsetU32;
    if (m_index->vectors() == nullptr)
        return groove_io::kInvalidOffsetU32;
    auto begin = m_index->vectors()->cbegin();
    auto end = m_index->vectors()->cend();

    auto result = std::lower_bound(begin, end, page_id, comp_before);
    if (result != end) {
        // if all elements are greater than or equal to pageId then there is no vector
        // before so return invalid, otherwise return the element index prior to result.
        if (result == begin)
            return kInvalidOffsetU32;
        return std::distance(m_index->vectors()->cbegin(), --result);
    } else {
        // if there are no elements greater or equal to pageId, but vector is not empty,
        // then return the index of the last element in the vector, otherwise invalid.
        if (m_index->vectors()->size() > 0)
            return m_index->vectors()->size() - 1;
        return kInvalidOffsetU32;
    }
}

static bool comp_after(std::string_view lhs, const gii1::VectorDescriptor* vector)
{
    auto rhs = vector->page_id()->string_view();
    return lhs < rhs;
}

tu_uint32
groove_io::internal::IndexReader::findVectorAfter(const std::string_view page_id) const
{
    if (m_index == nullptr)
        return kInvalidOffsetU32;
    if (m_index->vectors() == nullptr)
        return kInvalidOffsetU32;
    auto begin = m_index->vectors()->cbegin();
    auto end = m_index->vectors()->cend();

    // if there is an element strictly greater than pageId then return the element index,
    // otherwise return invalid.
    auto result = std::upper_bound(begin, end, page_id, comp_after);
    if (result == end)
        return kInvalidOffsetU32;
    return std::distance(m_index->vectors()->cbegin(), result);
}

uint32_t
groove_io::internal::IndexReader::numVectors() const
{
    if (m_index == nullptr)
        return 0;
    return m_index->vectors()? m_index->vectors()->size() : 0;
}

const gii1::FrameDescriptor *
groove_io::internal::IndexReader::getFrame(uint32_t index) const
{
    if (m_index == nullptr)
        return nullptr;
    if (m_index->frames() && index < m_index->frames()->size())
        return m_index->frames()->Get(index);
    return nullptr;
}

uint32_t
groove_io::internal::IndexReader::numFrames() const
{
    if (m_index == nullptr)
        return 0;
    return m_index->frames()? m_index->frames()->size() : 0;
}

std::span<const tu_uint8>
groove_io::internal::IndexReader::bytesView() const
{
    return m_bytes;
}

std::string
groove_io::internal::IndexReader::dumpJson() const
{
    flatbuffers::Parser parser;
    parser.Parse((const char *) groove_io::schema::index::data);
    parser.opts.strict_json = true;

    std::string jsonData;
    auto *err = GenText(parser, m_bytes.data(), &jsonData);
    TU_ASSERT (err == nullptr);
    return jsonData;
}
