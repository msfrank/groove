#ifndef GROOVE_MODEL_MODEL_TYPES_H
#define GROOVE_MODEL_MODEL_TYPES_H

#include <arrow/builder.h>

#include <groove_data/category_builder.h>
#include <groove_data/data_types.h>

namespace groove_model {

    constexpr uint32_t kInvalidOffsetU32      = 0xFFFFFFFF;

    enum class SchemaVersion {
        Unknown,
        Version1,
    };

    enum class ValueType {
        Invalid,
        Nil,
        Bool,
        Int64,
        Float64,
        UInt64,
        UInt32,
        UInt16,
        UInt8,
        String,
        Attr,
        Element,
    };

    enum class ModelKeyType {
        Invalid,
        Category,
        Double,
        Int64,
    };

    enum class ModelKeyCollation {
        Invalid,
        Sorted,
        Indexed,
    };

    enum class ColumnValueType {
        Invalid,
        Double,
        Int64,
        String,
    };

    enum class ColumnValueFidelity {
        Invalid,
        OnlyValidValue,                             // row must have a valid value
        OnlyValidOrEmpty,                           // row value must be valid or empty
        AnyFidelityAllowed,                         // row value can have any fidelity state
    };

    struct Resource {
        tu_int16 nsKey;
        tu_uint32 idValue;
    };

    class PageData {
    public:
        virtual ~PageData() = default;
        virtual const char *getData() const = 0;
        virtual tu_uint32 getSize() const = 0;
    };

    class DataDef {

    public:
        DataDef();
        DataDef(groove_data::DataKeyType key, groove_data::DataValueType value);
        DataDef(const DataDef &other);

        groove_data::DataKeyType getKey() const;
        groove_data::DataValueType getValue() const;

    private:
        groove_data::DataKeyType m_key;
        groove_data::DataValueType m_value;
    };

    template <typename DefType, groove_data::CollationMode collation>
    struct PageTraits {};

    template <typename DefType, groove_data::CollationMode collation>
    struct ColumnTraits {};

    struct CategoryDouble : public DataDef {
        using KeyType = groove_data::Category;
        using ValueType = double;
        using KeyBuilderType = groove_data::CategoryBuilder;
        using ValueBuilderType = arrow::DoubleBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_CATEGORY; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_DOUBLE; };

        CategoryDouble();
    };

    struct CategoryInt64 : public DataDef {
        using KeyType = groove_data::Category;
        using ValueType = tu_int64;
        using KeyBuilderType = groove_data::CategoryBuilder;
        using ValueBuilderType = arrow::Int64Builder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_CATEGORY; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_INT64; };

        CategoryInt64();
    };

    struct CategoryString : public DataDef {
        using KeyType = groove_data::Category;
        using ValueType = std::string;
        using KeyBuilderType = groove_data::CategoryBuilder;
        using ValueBuilderType = arrow::StringBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_CATEGORY; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_STRING; };

        CategoryString();
    };

    struct DoubleDouble : public DataDef {
        using KeyType = double;
        using ValueType = double;
        using KeyBuilderType = arrow::DoubleBuilder;
        using ValueBuilderType = arrow::DoubleBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_DOUBLE; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_DOUBLE; };

        DoubleDouble();
    };

    struct DoubleInt64 : public DataDef {
        using KeyType = double;
        using ValueType = tu_int64;
        using KeyBuilderType = arrow::DoubleBuilder;
        using ValueBuilderType = arrow::Int64Builder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_DOUBLE; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_INT64; };

        DoubleInt64();
    };

    struct DoubleString : public DataDef {
        using KeyType = double;
        using ValueType = std::string;
        using KeyBuilderType = arrow::DoubleBuilder;
        using ValueBuilderType = arrow::StringBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_DOUBLE; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_STRING; };

        DoubleString();
    };

    struct Int64Double : public DataDef {
        using KeyType = tu_int64;
        using ValueType = double;
        using KeyBuilderType = arrow::Int64Builder;
        using ValueBuilderType = arrow::DoubleBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_INT64; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_DOUBLE; };

        Int64Double();
    };

    struct Int64Int64 : public DataDef {
        using KeyType = tu_int64;
        using ValueType = tu_int64;
        using KeyBuilderType = arrow::Int64Builder;
        using ValueBuilderType = arrow::Int64Builder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_INT64; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_INT64; };

        Int64Int64();
    };

    struct Int64String : public DataDef {
        using KeyType = tu_int64;
        using ValueType = std::string;
        using KeyBuilderType = arrow::Int64Builder;
        using ValueBuilderType = arrow::StringBuilder;

        static constexpr groove_data::DataKeyType static_key_type() {
            return groove_data::DataKeyType::KEY_INT64; };
        static constexpr groove_data::DataValueType static_value_type() {
            return groove_data::DataValueType::VALUE_TYPE_STRING; };

        Int64String();
    };

    struct NamespaceAddress {
    public:
        NamespaceAddress() : u32(kInvalidOffsetU32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct AttrAddress {
    public:
        AttrAddress() : u32(kInvalidOffsetU32) {};
        explicit AttrAddress(tu_uint32 u32) : u32(u32) {};
        AttrAddress(const AttrAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const AttrAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct ColumnAddress {
    public:
        ColumnAddress() : u32(kInvalidOffsetU32) {};
        explicit ColumnAddress(tu_uint32 u32) : u32(u32) {};
        ColumnAddress(const ColumnAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ColumnAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct ModelAddress {
    public:
        ModelAddress() : u32(kInvalidOffsetU32) {};
        explicit ModelAddress(tu_uint32 u32) : u32(u32) {};
        ModelAddress(const ModelAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ModelAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct AttrId {
        AttrId();
        AttrId(const NamespaceAddress &address, tu_uint32 type);
        AttrId(const AttrId &other);

        NamespaceAddress getAddress() const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            return H::combine(std::move(h), id.m_address.getAddress(), id.m_type);
        }

    private:
        NamespaceAddress m_address;
        tu_uint32 m_type;
    };

    // forward declarations
    namespace internal {
        class SchemaReader;
    }
}

#endif // GROOVE_MODEL_MODEL_TYPES_H