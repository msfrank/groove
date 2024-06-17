#ifndef GROOVE_IO_INDEX_WALKER_H
#define GROOVE_IO_INDEX_WALKER_H

#include <groove_model/page_id.h>
#include <tempo_utils/integer_types.h>

#include "frame_walker.h"
#include "vector_walker.h"

namespace groove_io {

    class IndexWalker {

    public:
        IndexWalker();
        IndexWalker(const IndexWalker &other);

        bool isValid() const;

        VectorWalker getVector(tu_uint32 index) const;
        VectorWalker findVector(const groove_model::PageId &pageId) const;
        VectorWalker findVectorBefore(const groove_model::PageId &pageId) const;
        VectorWalker findVectorAfter(const groove_model::PageId &pageId) const;
        tu_uint32 numVectors() const;

        FrameWalker getFrame(tu_uint32 index) const;
        tu_uint32 numFrames() const;

    private:
        std::shared_ptr<const internal::IndexReader> m_reader;

        IndexWalker(std::shared_ptr<const internal::IndexReader> reader);
        friend class GrooveIndex;
    };
}

#endif // GROOVE_IO_INDEX_WALKER_H
