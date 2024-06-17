
#include <groove_model/conversion_utils.h>
#include <tempo_utils/big_endian.h>

std::string
groove_model::int64_to_bytes(tu_int64 i64)
{
    i64 = H_TO_BE64(i64);
    auto *dst = reinterpret_cast<const char *>(&i64);
    std::string bytes;
    bytes.append(dst, 8);
    return bytes;
}

std::string
groove_model::double_to_bytes(double dbl)
{
    // get the raw bytes of the double
    auto *src = reinterpret_cast<tu_uint64 *>(&dbl);

    // if the native byte order is little endian, then convert to big endian
    tu_uint64 u64 = H_TO_BE64(*src);
    char *dst = reinterpret_cast<char *>(&u64);

    // if the double is negative, then take the twos complement
    if (dbl < 0) {
        bool flip = false;
        for (int index = 7; index >= 0; index--) {
            if (flip) {
                dst[index] = ~dst[index];
            } else {
                for (int shift = 0; shift < 8; shift++) {
                    uint8_t bit = 0x01 << shift;
                    if (!flip) {
                        if ((dst[index] & bit) > 0)
                            flip = true;
                    } else {
                        uint8_t mask = 0xFF << shift;
                        dst[index] ^= mask;
                        break;
                    }
                }
            }
        }
    }

    std::string bytes;
    bytes.append(dst, 8);
    return bytes;
}

std::string
groove_model::key_to_bytes(Option<double> key)
{
    if (key.isEmpty())
        return std::string("\0");
    double dbl = key.getValue();
    if (dbl < 0)
        return absl::StrCat("n", double_to_bytes(dbl));
    return absl::StrCat("p", double_to_bytes(dbl));
}

std::string
groove_model::key_to_bytes(Option<tu_int64> key)
{
    if (key.isEmpty())
        return std::string("\0");
    tu_int64 i64 = key.getValue();
    if (i64 < 0)
        return absl::StrCat("n", int64_to_bytes(i64));
    return absl::StrCat("p", int64_to_bytes(i64));
}

std::string
groove_model::key_to_bytes(const Option<groove_data::Category> key)
{
    if (key.isEmpty())
        return std::string("\0");
    auto cat = key.getValue();
    std::string rangeKey;
    union {
        tu_uint32 size;
        char bytes[4];
    } dst;

    if (cat.size() == 0) {              // special case if key is an empty string list
        dst.size = 0;
        rangeKey.append(dst.bytes, 4);
    } else {                            // otherwise loop appending length-prefixed strings
        for (auto iterator = cat.cbegin(); iterator != cat.cend(); iterator++) {
            auto part = *iterator;
            dst.size = H_TO_BE32(part->size());
            rangeKey.append(dst.bytes, 4);
            rangeKey.append(part->data(), part->size());
        }
    }
    return rangeKey;
}
