
#include <groove_model/generated/schema.h>
#include <groove_model/internal/schema_reader.h>
#include <groove_model/groove_schema.h>

groove_model::GrooveSchema::GrooveSchema()
{
}

groove_model::GrooveSchema::GrooveSchema(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::SchemaReader>(bytes);
}

groove_model::GrooveSchema::GrooveSchema(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::SchemaReader>(unownedBytes);
}

groove_model::GrooveSchema::GrooveSchema(const groove_model::GrooveSchema &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
groove_model::GrooveSchema::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_model::SchemaVersion
groove_model::GrooveSchema::getABI() const
{
    if (m_reader == nullptr)
        return SchemaVersion::Unknown;
    switch (m_reader->getABI()) {
        case gms1::SchemaVersion::Version1:
            return SchemaVersion::Version1;
        case gms1::SchemaVersion::Unknown:
        default:
            return SchemaVersion::Unknown;
    }
}

groove_model::SchemaWalker
groove_model::GrooveSchema::getSchema() const
{
    if (m_reader == nullptr)
        return {};
    return SchemaWalker(m_reader);
}

std::shared_ptr<const groove_model::internal::SchemaReader>
groove_model::GrooveSchema::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
groove_model::GrooveSchema::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

/**
 * Verify that the given span of bytes is a valid schema.
 *
 * @param bytes The span of bytes containing a schema.
 * @param noIdentifier If true, then there is no identifier sequence preceding the data.
 * @return true if bytes refers to a valid schema, otherwise false.
 */
bool
groove_model::GrooveSchema::verify(std::span<const tu_uint8> bytes, bool noIdentifier)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    if (noIdentifier)
        return verifier.VerifyBuffer<gms1::Schema>();
    return verifier.VerifyBuffer<gms1::Schema>(gms1::SchemaIdentifier());
}
