#ifndef GROOVE_IO_FRAME_WALKER_H
#define GROOVE_IO_FRAME_WALKER_H

#include <groove_model/page_id.h>

#include "io_types.h"

namespace groove_io {

    class FrameWalker {

    public:
        FrameWalker();
        FrameWalker(const FrameWalker &other);

        bool isValid() const;

        tu_uint32 getIndex() const;

        tu_uint32 getKeyOffset() const;
        tu_uint32 getFrameOffset() const;
        tu_uint32 getFrameSize() const;

    private:
        std::shared_ptr<const internal::IndexReader> m_reader;
        tu_uint32 m_index;

        FrameWalker(std::shared_ptr<const internal::IndexReader> reader, tu_uint32 index);
        friend class IndexWalker;
    };
}

#endif // GROOVE_IO_FRAME_WALKER_H
