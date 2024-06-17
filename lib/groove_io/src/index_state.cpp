
#include <groove_io/index_state.h>
#include <tempo_utils/memory_bytes.h>

groove_io::IndexState::IndexState()
{
}

tempo_utils::Result<groove_io::IndexVector *>
groove_io::IndexState::appendVector(
    const groove_model::PageId &pageId,
    FrameAddress frameAddress,
    tu_uint32 valOffset)
{
    TU_ASSERT (pageId.isValid());
    TU_ASSERT (valOffset != kInvalidOffsetU32);

    if (m_vectorIndex.contains(pageId))
        return IOStatus::forCondition(
            IOCondition::kIOInvariant, "frame already exists with specified page id");

    auto index = m_indexVectors.size();

    auto *vector = new IndexVector(pageId, frameAddress, valOffset, kInvalidOffsetU32);
    m_indexVectors.push_back(vector);
    m_vectorIndex[pageId] = index;
    return vector;
}

tempo_utils::Result<groove_io::IndexVector *>
groove_io::IndexState::appendVector(
    const groove_model::PageId &pageId,
    FrameAddress frameAddress,
    tu_uint32 valOffset,
    tu_uint32 fidOffset)
{
    TU_ASSERT (pageId.isValid());
    TU_ASSERT (valOffset != kInvalidOffsetU32);
    TU_ASSERT (fidOffset != kInvalidOffsetU32);

    if (m_vectorIndex.contains(pageId))
        return IOStatus::forCondition(
            IOCondition::kIOInvariant, "frame already exists with specified page id");

    auto index = m_indexVectors.size();

    auto *vector = new IndexVector(pageId, frameAddress, valOffset, fidOffset);
    m_indexVectors.push_back(vector);
    m_vectorIndex[pageId] = index;
    return vector;
}

tempo_utils::Result<groove_io::IndexFrame *>
groove_io::IndexState::appendFrame(
    tu_uint32 keyOffset,
    tu_uint32 frameOffset,
    tu_uint32 frameSize)
{
    FrameAddress address(m_indexFrames.size());
    auto *frame = new IndexFrame(address, keyOffset, frameOffset, frameSize);
    m_indexFrames.push_back(frame);
    return frame;
}

tempo_utils::Result<groove_io::GrooveIndex>
groove_io::IndexState::toIndex(bool noIdentifier) const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<gii1::VectorDescriptor>> vectors_vector;
    std::vector<flatbuffers::Offset<gii1::FrameDescriptor>> frames_vector;

    // serialize vectors
    for (const auto &vector : m_indexVectors) {
        vectors_vector.push_back(gii1::CreateVectorDescriptor(buffer,
            buffer.CreateString(vector->getPageId().getBytes()),
            vector->getFrameAddress().getAddress(),
            vector->getValOffset(), vector->getFidOffset()));
    }
    auto fb_vectors = buffer.CreateVectorOfSortedTables(&vectors_vector);

    // serialize frames
    for (const auto &frame : m_indexFrames) {
        frames_vector.push_back(gii1::CreateFrameDescriptor(buffer,
            frame->getKeyOffset(), frame->getFrameOffset(), frame->getFrameSize()));
    }
    auto fb_frames = buffer.CreateVector(frames_vector);

    // build index from buffer
    gii1::IndexBuilder indexBuilder(buffer);

    indexBuilder.add_abi(gii1::IndexVersion::Version1);
    indexBuilder.add_vectors(fb_vectors);
    indexBuilder.add_frames(fb_frames);

    // serialize index and mark the buffer as finished
    auto index = indexBuilder.Finish();
    if (noIdentifier) {
        buffer.Finish(index);
    } else {
        buffer.Finish(index, gii1::IndexIdentifier());
    }

    // copy the flatbuffer into our own byte array and instantiate index
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return GrooveIndex(bytes);
}