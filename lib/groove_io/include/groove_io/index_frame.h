#ifndef GROOVE_IO_INDEX_FRAME_H
#define GROOVE_IO_INDEX_FRAME_H

#include "io_types.h"

namespace groove_io {

    class IndexFrame {
    public:
        IndexFrame(FrameAddress address, tu_uint32 keyOffset, tu_uint32 frameOffset, tu_uint32 frameSize);

        FrameAddress getAddress() const;

        tu_uint32 getKeyOffset() const;
        tu_uint32 getFrameOffset() const;
        tu_uint32 getFrameSize() const;

    private:
        FrameAddress m_address;
        tu_uint32 m_keyOffset;
        tu_uint32 m_frameOffset;
        tu_uint32 m_frameSize;
    };
}

#endif // GROOVE_IO_INDEX_FRAME_H