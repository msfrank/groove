#ifndef GROOVE_DATA_DATA_TYPES_H
#define GROOVE_DATA_DATA_TYPES_H

#include <tempo_utils/option_template.h>

#include "category.h"

namespace groove_data {

    enum class DataKeyType {
        INVALID,
        KEY_UNKNOWN,
        KEY_CATEGORY,
        KEY_DOUBLE,
        KEY_INT64,
    };

    enum class DataValueType {
        INVALID,
        VALUE_TYPE_UNKNOWN,
        VALUE_TYPE_DOUBLE,
        VALUE_TYPE_INT64,
        VALUE_TYPE_STRING,
    };

    enum class CollationMode {
        INVALID,
        COLLATION_UNKNOWN,          // unknown collation.  it is an error to send this over the wire
        COLLATION_SORTED,           // data is sorted, but there may be duplicate keys
        COLLATION_INDEXED,          // data is sorted and all keys are unique
    };

    enum class DatumFidelity {
        INVALID,
        FIDELITY_UNKNOWN,           // unknown fidelity. it is an error  to send this over the wire
        FIDELITY_NODATA,            // no value will ever exist for this key (is final)
        FIDELITY_VALID,             // data exists for this key, is final and is exact
        FIDELITY_APPROXIMATE,       // data exists for this key, is final but is not exact
        FIDELITY_MISSING,           // data is missing but may be determined later (is not final)
        FIDELITY_INVALID,           // data exists but is not representable
    };

    enum class DataVectorType {
        VECTOR_TYPE_UNKNOWN = 0,
        VECTOR_TYPE_DOUBLE_DOUBLE,
        VECTOR_TYPE_DOUBLE_INT64,
        VECTOR_TYPE_DOUBLE_STRING,
        VECTOR_TYPE_INT64_DOUBLE,
        VECTOR_TYPE_INT64_INT64,
        VECTOR_TYPE_INT64_STRING,
        VECTOR_TYPE_CATEGORY_DOUBLE,
        VECTOR_TYPE_CATEGORY_INT64,
        VECTOR_TYPE_CATEGORY_STRING
    };

    enum class DataFrameType {
        FRAME_TYPE_UNKNOWN = 0,
        FRAME_TYPE_DOUBLE,
        FRAME_TYPE_INT64,
        FRAME_TYPE_CATEGORY
    };

    struct DoubleRange {
        Option<double> start;
        bool start_exclusive = false;
        Option<double> end;
        bool end_exclusive = true;
    };

    struct Int64Range {
        Option<tu_int64> start;
        bool start_exclusive = false;
        Option<tu_int64> end;
        bool end_exclusive = true;
    };

    struct CategoryRange {
        Option<Category> start;
        bool start_exclusive = false;
        Option<Category> end;
        bool end_exclusive = true;
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, CategoryRange range);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, DoubleRange range);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, Int64Range range);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, DataVectorType t);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, DataFrameType t);

    /**
     *
     * @tparam VectorType
     * @param vector
     * @return
     */
    template <class VectorType>
    inline std::shared_ptr<VectorType> empty(std::shared_ptr<const VectorType> vector)
    {
        auto table = vector->getTable();
        auto view = table->Slice(0, 0);
        auto keyIndex = vector->getKeyFieldIndex();
        auto valIndex = vector->getValFieldIndex();
        auto fidIndex = vector->getFidFieldIndex();
        return VectorType::create(view, keyIndex, valIndex, fidIndex);
    }

    /**
     * if key is present in vector, then the index of the element matching key is returned.
     * otherwise if key is not present in vector, then -1 is returned.
     *
     * @tparam Vector
     * @tparam Key
     * @param vector
     * @param key
     * @return
     */
    template <class VectorType, class KeyType>
    inline int
    search_indexed_vector(std::shared_ptr<VectorType> vector, const KeyType &key)
    {
        const tu_int64 size = vector->getSize();
        tu_int64 l = 0;
        tu_int64 r = size - 1;

        while (l != r) {
            const tu_int64 m = std::ceil(static_cast<double>(l + r) / 2.0);
            const auto t = vector->getDatum(m);
            if (t.key > key) {
                r = m - 1;
            } else {
                l = m;
            }
        }

        const auto t = vector->getDatum(l);
        return t.key == key? l : -1;
    }

    /**
     * if key is present in vector, then the index of the leftmost element matching key is
     * returned and found is set to true. otherwise if key is not present in vector, then found
     * is set to false and the index of the lower bound (the smallest element greater than key)
     * is returned. if all elements are larger than key, then the lower bound is 0. if all
     * elements are smaller than key, then the size of the vector is returned.
     *
     * @tparam Vector
     * @tparam Key
     * @param vector
     * @param key
     * @return
     */
    template <class VectorType, class KeyType>
    inline int
    find_vector_lower_bound(std::shared_ptr<VectorType> vector, const KeyType &key, bool &found)
    {
        const tu_int64 size = vector->getSize();
        tu_int64 l = 0;
        tu_int64 r = size - 1;

        found = false;
        while (l < r) {
            const tu_int64 m = std::floor(static_cast<double>(l + r) / 2.0);
            const auto t = vector->getDatum(m);
            if (t.key < key) {
                l = m + 1;
            } else {
                r = m;
            }
        }

        if (l < size) {
            const auto t = vector->getDatum(l);
            if (t.key == key) {
                found = true;
            }
        }

        return l;
    }

    /**
     * if key is present in vector, then the index of the rightmost element matching key is
     * returned and found is set to true. otherwise if key is not present in vector, then found
     * is set to false and the index of the upper bound (the largest element smaller than key)
     * is returned. if all elements are larger than key, then the upper bound is -1. if all
     * elements are smaller than key, then the index of the last element in the vector is returned.
     *
     * @tparam Vector
     * @tparam Key
     * @param vector
     * @param key
     * @return
     */
    template <class VectorType, class KeyType>
    inline int
    find_vector_upper_bound(std::shared_ptr<VectorType> vector, const KeyType &key, bool &found)
    {
        const tu_int64 size = vector->getSize();
        tu_int64 l = 0;
        tu_int64 r = size;

        found = false;
        while (l < r) {
            const tu_int64 m = std::floor(static_cast<double>(l + r) / 2.0);
            const auto t = vector->getDatum(m);
            if (t.key > key) {
                r = m;
            } else {
                l = m + 1;
            }
        }

        if (r > 0) {
            const auto t = vector->getDatum(r - 1);
            if (t.key == key) {
                found = true;
            }
        }

        return r - 1;
    }

    /**
     *
     * @tparam VectorType
     * @tparam RangeType
     * @param vector
     * @param range
     * @return
     */
    template <class VectorType, class RangeType>
    inline int
    find_start_index(std::shared_ptr<const VectorType> vector, const RangeType &range)
    {
        if (range.start.isEmpty())
            return 0;

        if (!range.start_exclusive) {
            bool found;
            int index = groove_data::find_vector_lower_bound(vector, range.start.getValue(), found);
            if (!found && index == vector->getSize())
                return -1;
            return index;
        }

        bool found;
        int index = groove_data::find_vector_upper_bound(vector, range.start.getValue(), found);
        if (!found && index == vector->getSize() - 1)
            return -1;

        //
        index++;
        if (index == vector->getSize())
            return -1;

        return index;
    }

    /**
     *
     * @tparam VectorType
     * @tparam RangeType
     * @param vector
     * @param range
     * @return
     */
    template <class VectorType, class RangeType>
    inline int
    find_end_index(std::shared_ptr<const VectorType> vector, const RangeType &range)
    {
        if (range.end.isEmpty())
            return vector->getSize() - 1;

        if (!range.end_exclusive) {
            bool found;
            int index = groove_data::find_vector_upper_bound(vector, range.end.getValue(), found);
            if (!found && index == -1)
                return -1;
            return index;
        }

        bool found;
        int index = groove_data::find_vector_lower_bound(vector, range.start.getValue(), found);
        if (!found && index == 0)
            return -1;

        //
        index--;
        if (index < 0)
            return -1;

        return index;
    }
}

#endif // GROOVE_DATA_DATA_TYPES_H