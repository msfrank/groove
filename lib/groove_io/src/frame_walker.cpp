
#include <groove_io/frame_walker.h>
#include <groove_io/generated/index.h>
#include <groove_io/internal/index_reader.h>

groove_io::FrameWalker::FrameWalker()
{
}

groove_io::FrameWalker::FrameWalker(std::shared_ptr<const internal::IndexReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

groove_io::FrameWalker::FrameWalker(const FrameWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
groove_io::FrameWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

tu_uint32
groove_io::FrameWalker::getIndex() const
{
    return m_index;
}

tu_uint32
groove_io::FrameWalker::getKeyOffset() const
{
    if (!isValid())
        return 0;
    auto *frame = m_reader->getFrame(m_index);
    if (frame == nullptr)
        return 0;
    return frame->key_offset();
}

tu_uint32
groove_io::FrameWalker::getFrameOffset() const
{
    if (!isValid())
        return 0;
    auto *frame = m_reader->getFrame(m_index);
    if (frame == nullptr)
        return 0;
    return frame->frame_offset();
}

tu_uint32
groove_io::FrameWalker::getFrameSize() const
{
    if (!isValid())
        return 0;
    auto *frame = m_reader->getFrame(m_index);
    if (frame == nullptr)
        return 0;
    return frame->frame_size();
}