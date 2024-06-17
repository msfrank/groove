#ifndef GROOVE_IO_INDEX_VECTOR_H
#define GROOVE_IO_INDEX_VECTOR_H

#include <groove_model/page_id.h>

#include "io_types.h"

namespace groove_io {

    class IndexVector {
    public:
        IndexVector(
            const groove_model::PageId &pageId,
            FrameAddress frameAddress,
            tu_uint32 valOffset,
            tu_uint32 fidOffset);

        groove_model::PageId getPageId() const;
        FrameAddress getFrameAddress() const;
        tu_uint32 getValOffset() const;
        tu_uint32 getFidOffset() const;

    private:
        groove_model::PageId m_pageId;
        FrameAddress m_frameAddress;
        tu_uint32 m_valOffset;
        tu_uint32 m_fidOffset;
    };
}

#endif // GROOVE_IO_INDEX_VECTOR_H