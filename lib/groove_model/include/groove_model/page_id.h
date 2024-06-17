#ifndef GROOVE_MODEL_PAGE_ID_H
#define GROOVE_MODEL_PAGE_ID_H

#include <string>

#include <absl/hash/hash.h>

#include <groove_data/category.h>
#include <tempo_utils/integer_types.h>
#include <tempo_utils/option_template.h>
#include <tempo_utils/logging.h>
#include <tempo_utils/url.h>

#include "conversion_utils.h"
#include "model_types.h"

namespace groove_model {

    /**
     *
     * @param collation
     * @return
     */
    constexpr char collation_to_byte(groove_data::CollationMode collation) {
        switch (collation) {
            case groove_data::CollationMode::COLLATION_SORTED:
                return 's';
            case groove_data::CollationMode::COLLATION_INDEXED:
                return 'i';
            default:
                return '\0';
        }
    }

    constexpr char key_type_to_byte(groove_data::DataKeyType key) {
        switch (key) {
            case groove_data::DataKeyType::KEY_DOUBLE:
                return 'd';
            case groove_data::DataKeyType::KEY_INT64:
                return 'i';
            case groove_data::DataKeyType::KEY_CATEGORY:
                return 'c';
            default:
                return '\0';
        }
    }

    constexpr char value_type_to_byte(groove_data::DataValueType value) {
        switch (value) {
            case groove_data::DataValueType::VALUE_TYPE_DOUBLE:
                return 'd';
            case groove_data::DataValueType::VALUE_TYPE_INT64:
                return 'i';
            case groove_data::DataValueType::VALUE_TYPE_STRING:
                return 's';
            default:
                return '\0';
        }
    }

    class PageId {
    public:
        PageId();

        bool isValid() const;

        std::string getPrefix() const;
        std::string getKey() const;
        std::string getBytes() const;
        groove_data::CollationMode getCollation() const;
        groove_data::DataKeyType getKeyType() const;
        groove_data::DataValueType getValueType() const;

        bool operator<(const PageId &other) const;
        bool operator<=(const PageId &other) const;
        bool operator>(const PageId &other) const;
        bool operator>=(const PageId &other) const;
        bool operator==(const PageId &other) const;
        bool operator!=(const PageId &other) const;

        static PageId fromString(std::string_view s);

        static PageId create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            groove_data::DataValueType valueType,
            groove_data::CollationMode collation,
            Option<groove_data::Category> key);
        static PageId create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            groove_data::DataValueType valueType,
            groove_data::CollationMode collation,
            Option<double> key);
        static PageId create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            groove_data::DataValueType valueType,
            groove_data::CollationMode collation,
            Option<tu_int64> key);

        template <typename H>
        friend H AbslHashValue(H h, const PageId &pageId) {
            TU_ASSERT (pageId.m_id != nullptr);
            // avoid a string copy
            std::string s(pageId.m_id->data(), pageId.m_id->size());
            return H::combine(std::move(h), s);
        }

    private:
        std::shared_ptr<const std::string> m_id;
        std::pair<int,int> m_prefixPart;
        std::pair<int,int> m_typePart;
        std::pair<int,int> m_keyPart;

        PageId(
            std::shared_ptr<const std::string> id,
            std::pair<int,int> m_prefixPart,
            std::pair<int,int> m_typePart,
            std::pair<int,int> m_keyPart);

        static PageId create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            groove_data::DataKeyType keyType,
            groove_data::DataValueType valueType,
            groove_data::CollationMode collation,
            const std::string &keyBytes);

    public:

        /**
         *
         * @tparam DefType
         * @tparam collation
         * @tparam KeyType
         * @param modelId
         * @param columnId
         * @param key
         * @return
         */
        template <typename DefType, groove_data::CollationMode collation,
            typename KeyType = typename DefType::KeyType>
        static PageId
        create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            Option<KeyType> key)
        {
//            TU_ASSERT (datasetUrl.isValid());
//            TU_ASSERT (modelId != nullptr && !modelId->empty());
//            TU_ASSERT (columnId != nullptr && !columnId->empty());
//
//            std::string idBytes;
//            std::pair<int,int> prefixPart;
//            std::pair<int,int> typePart;
//            std::pair<int,int> keyPart;
//
//            absl::StrAppend(&idBytes, "page\x1f", datasetUrl.toString(), "\x1f", *modelId, "\x1f", *columnId);
//            idBytes.push_back('\x1e');
//            prefixPart.first = 0;
//            prefixPart.second = idBytes.size();
//
//            idBytes.push_back(collation_to_byte(collation));
//            idBytes.push_back(key_type_to_byte(DefType::static_key_type()));
//            idBytes.push_back(value_type_to_byte(DefType::static_value_type()));
//            idBytes.push_back('\x1e');
//            typePart.first = prefixPart.second;
//            typePart.second = 4;
//
//            auto keyBytes = key_to_bytes(key);
//            absl::StrAppend(&idBytes, keyBytes);
//            keyPart.first = typePart.first + typePart.second;
//            keyPart.second = keyBytes.size();
//
//            auto id = std::make_shared<const std::string>(idBytes);
//            return PageId(id, prefixPart, typePart, keyPart);
            return create(datasetUrl, modelId, columnId,
                DefType::static_key_type(), DefType::static_value_type(), collation,
                key_to_bytes(key));
        }
    };
}

#endif // GROOVE_MODEL_PAGE_ID_H