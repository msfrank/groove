
#include <groove_io/generated/index.h>
#include <groove_io/groove_index.h>
#include <groove_io/internal/index_reader.h>

groove_io::GrooveIndex::GrooveIndex()
{
}

groove_io::GrooveIndex::GrooveIndex(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::IndexReader>(bytes);
}

groove_io::GrooveIndex::GrooveIndex(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::IndexReader>(unownedBytes);
}

groove_io::GrooveIndex::GrooveIndex(const groove_io::GrooveIndex &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
groove_io::GrooveIndex::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_io::IndexVersion
groove_io::GrooveIndex::getABI() const
{
    if (!isValid())
        return IndexVersion::Unknown;
    switch (m_reader->getABI()) {
        case gii1::IndexVersion::Version1:
            return IndexVersion::Version1;
        case gii1::IndexVersion::Unknown:
        default:
            return IndexVersion::Unknown;
    }
}

groove_io::IndexWalker
groove_io::GrooveIndex::getIndex() const
{
    if (!isValid())
        return {};
    return IndexWalker(m_reader);
}

std::span<const tu_uint8>
groove_io::GrooveIndex::bytesView() const
{
    if (!isValid())
        return {};
    return m_reader->bytesView();
}

/**
 * Verify that the given span of bytes is a valid index.
 *
 * @param bytes The span of bytes containing an index.
 * @param noIdentifier If true, then there is no identifier sequence preceding the data.
 * @return true if bytes refers to a valid index, otherwise false.
 */
bool
groove_io::GrooveIndex::verify(std::span<const tu_uint8> bytes, bool noIdentifier)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    if (noIdentifier)
        return verifier.VerifyBuffer<gii1::Index>();
    return verifier.VerifyBuffer<gii1::Index>(gii1::IndexIdentifier());
}