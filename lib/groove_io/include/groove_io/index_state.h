#ifndef GROOVE_IO_INDEX_WRITER_H
#define GROOVE_IO_INDEX_WRITER_H

#include <absl/container/flat_hash_map.h>

#include <groove_model/page_id.h>
#include <tempo_utils/integer_types.h>

#include "groove_index.h"
#include "index_frame.h"
#include "index_vector.h"
#include "io_result.h"

namespace groove_io {

    class IndexState {

    public:
        IndexState();

        tempo_utils::Result<IndexVector *> appendVector(
            const groove_model::PageId &pageId,
            FrameAddress frameAddress,
            tu_uint32 valOffset);
        tempo_utils::Result<IndexVector *> appendVector(
            const groove_model::PageId &pageId,
            FrameAddress frameAddress,
            tu_uint32 valOffset,
            tu_uint32 fidOffset);

        tempo_utils::Result<IndexFrame *> appendFrame(
            tu_uint32 keyOffset,
            tu_uint32 frameOffset,
            tu_uint32 frameSize);

        tempo_utils::Result<GrooveIndex> toIndex(bool noIdentifier = false) const;

    private:
        std::vector<IndexVector *> m_indexVectors;
        std::vector<IndexFrame *> m_indexFrames;
        absl::flat_hash_map<groove_model::PageId,tu_uint32> m_vectorIndex;
    };
}

#endif // GROOVE_IO_INDEX_WRITER_H