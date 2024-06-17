
#include <groove_io/generated/index.h>
#include <groove_io/index_walker.h>
#include <groove_io/internal/index_reader.h>

groove_io::IndexWalker::IndexWalker()
{
}

groove_io::IndexWalker::IndexWalker(std::shared_ptr<const internal::IndexReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

groove_io::IndexWalker::IndexWalker(const IndexWalker &other)
    : m_reader(other.m_reader)
{
}

bool
groove_io::IndexWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_io::VectorWalker
groove_io::IndexWalker::getVector(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numVectors() <= index)
        return {};
    return VectorWalker(m_reader, index);
}

groove_io::VectorWalker
groove_io::IndexWalker::findVector(const groove_model::PageId &pageId) const
{
    if (!isValid())
        return {};
    auto pageIdBytes = pageId.getBytes();
    auto index = m_reader->findVector(pageIdBytes);
    if (index == kInvalidOffsetU32)
        return {};
    return VectorWalker(m_reader, index);
}

groove_io::VectorWalker
groove_io::IndexWalker::findVectorBefore(const groove_model::PageId &pageId) const
{
    if (!isValid())
        return {};
    auto pageIdBytes = pageId.getBytes();
    auto index = m_reader->findVectorBefore(pageIdBytes);
    if (index == kInvalidOffsetU32)
        return {};
    return VectorWalker(m_reader, index);
}

groove_io::VectorWalker
groove_io::IndexWalker::findVectorAfter(const groove_model::PageId &pageId) const
{
    if (!isValid())
        return {};
    auto pageIdBytes = pageId.getBytes();
    auto index = m_reader->findVectorAfter(pageIdBytes);
    if (index == kInvalidOffsetU32)
        return {};
    return VectorWalker(m_reader, index);
}

tu_uint32
groove_io::IndexWalker::numVectors() const
{
    if (!isValid())
        return {};
    return m_reader->numVectors();
}

groove_io::FrameWalker
groove_io::IndexWalker::getFrame(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numFrames() <= index)
        return {};
    return FrameWalker(m_reader, index);
}

tu_uint32
groove_io::IndexWalker::numFrames() const
{
    if (!isValid())
        return {};
    return m_reader->numFrames();
}