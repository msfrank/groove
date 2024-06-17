
#include <groove_io/index_frame.h>

groove_io::IndexFrame::IndexFrame(
    FrameAddress address,
    tu_uint32 keyOffset,
    tu_uint32 frameOffset,
    tu_uint32 frameSize)
    : m_address(address),
      m_keyOffset(keyOffset),
      m_frameOffset(frameOffset),
      m_frameSize(frameSize)
{
}

groove_io::FrameAddress
groove_io::IndexFrame::getAddress() const
{
    return m_address;
}

tu_uint32
groove_io::IndexFrame::getKeyOffset() const
{
    return m_keyOffset;
}

tu_uint32
groove_io::IndexFrame::getFrameOffset() const
{
    return m_frameOffset;
}

tu_uint32
groove_io::IndexFrame::getFrameSize() const
{
    return m_frameSize;
}