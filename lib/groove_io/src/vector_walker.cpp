
#include <groove_io/generated/index.h>
#include <groove_io/internal/index_reader.h>
#include <groove_io/vector_walker.h>

groove_io::VectorWalker::VectorWalker()
{
}

groove_io::VectorWalker::VectorWalker(std::shared_ptr<const internal::IndexReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

groove_io::VectorWalker::VectorWalker(const VectorWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
groove_io::VectorWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

groove_model::PageId
groove_io::VectorWalker::getPageId() const
{
    if (!isValid())
        return {};
    auto *vector = m_reader->getVector(m_index);
    return groove_model::PageId::fromString(vector->page_id()->string_view());
}

tu_uint32
groove_io::VectorWalker::getFrameIndex() const
{
    if (!isValid())
        return {};
    auto *vector = m_reader->getVector(m_index);
    return vector->frame_index();
}

tu_uint32
groove_io::VectorWalker::getValOffset() const
{
    if (!isValid())
        return {};
    auto *vector = m_reader->getVector(m_index);
    return vector->val_offset();
}

tu_uint32
groove_io::VectorWalker::getFidOffset() const
{
    if (!isValid())
        return {};
    auto *vector = m_reader->getVector(m_index);
    return vector->fid_offset();
}