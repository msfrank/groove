#ifndef GROOVE_IO_INTERNAL_INDEX_READER_H
#define GROOVE_IO_INTERNAL_INDEX_READER_H

#include <groove_io/generated/index.h>
#include <tempo_utils/integer_types.h>

namespace groove_io::internal {

    class IndexReader {

    public:
        IndexReader(std::span<const tu_uint8> bytes);

        bool isValid() const;

        gii1::IndexVersion getABI() const;

        const gii1::VectorDescriptor *getVector(uint32_t index) const;
        tu_uint32 findVector(std::string_view page_id) const;
        tu_uint32 findVectorBefore(std::string_view page_id) const;
        tu_uint32 findVectorAfter(std::string_view page_id) const;
        uint32_t numVectors() const;

        const gii1::FrameDescriptor *getFrame(uint32_t index) const;
        uint32_t numFrames() const;

        std::span<const tu_uint8> bytesView() const;

        std::string dumpJson() const;

    private:
        std::span<const tu_uint8> m_bytes;
        const gii1::Index *m_index;
    };
}

#endif // GROOVE_IO_INTERNAL_INDEX_READER_H
