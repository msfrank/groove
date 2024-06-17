
#include <groove_io/index_vector.h>

groove_io::IndexVector::IndexVector(
    const groove_model::PageId &pageId,
    FrameAddress frameAddress,
    tu_uint32 valOffset,
    tu_uint32 fidOffset)
    : m_pageId(pageId),
      m_frameAddress(frameAddress),
      m_valOffset(valOffset),
      m_fidOffset(fidOffset)
{
    TU_ASSERT (m_pageId.isValid());
}

groove_model::PageId
groove_io::IndexVector::getPageId() const
{
    return m_pageId;
}

groove_io::FrameAddress
groove_io::IndexVector::getFrameAddress() const
{
    return m_frameAddress;
}

tu_uint32
groove_io::IndexVector::getValOffset() const
{
    return m_valOffset;
}

tu_uint32
groove_io::IndexVector::getFidOffset() const
{
    return m_fidOffset;
}