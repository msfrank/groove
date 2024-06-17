#ifndef GROOVE_IO_VECTOR_WALKER_H
#define GROOVE_IO_VECTOR_WALKER_H

#include <groove_model/page_id.h>

#include "io_types.h"

namespace groove_io {

    class VectorWalker {

    public:
        VectorWalker();
        VectorWalker(const VectorWalker &other);

        bool isValid() const;

        groove_model::PageId getPageId() const;
        tu_uint32 getFrameIndex() const;
        tu_uint32 getValOffset() const;
        tu_uint32 getFidOffset() const;

    private:
        std::shared_ptr<const internal::IndexReader> m_reader;
        tu_uint32 m_index;

        VectorWalker(std::shared_ptr<const internal::IndexReader> reader, tu_uint32 index);
        friend class IndexWalker;
    };
}

#endif // GROOVE_IO_VECTOR_WALKER_H
