#ifndef GROOVE_IO_IO_TYPES_H
#define GROOVE_IO_IO_TYPES_H

#include <tempo_utils/integer_types.h>

namespace groove_io {

    constexpr tu_uint32 kInvalidOffsetU32 = 0xFFFFFFFF;

    enum class IndexVersion {
        Unknown,
        Version1,
    };

    struct FrameAddress {
    public:
        FrameAddress() : u32(kInvalidOffsetU32) {};
        explicit FrameAddress(tu_uint32 u32) : u32(u32) {};
        FrameAddress(const FrameAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const FrameAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    // forward declarations
    namespace internal {
        class IndexReader;
    }
}

#endif // GROOVE_IO_IO_TYPES_H
