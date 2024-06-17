
#include <absl/strings/str_cat.h>

#include <groove_model/page_id.h>
#include <tempo_utils/big_endian.h>

groove_model::PageId::PageId()
    : m_id(std::make_shared<const std::string>()),
      m_prefixPart{-1, -1},
      m_typePart{-1, -1},
      m_keyPart{-1, -1}
{
}

groove_model::PageId::PageId(
    std::shared_ptr<const std::string> id,
    std::pair<int,int> prefixPart,
    std::pair<int,int> typePart,
    std::pair<int,int> keyPart)
    : m_id(id),
      m_prefixPart(prefixPart),
      m_typePart(typePart),
      m_keyPart(keyPart)
{
    TU_ASSERT (m_id != nullptr);
    TU_ASSERT (m_prefixPart.first >= 0 && m_prefixPart.second >= 0);
    TU_ASSERT (m_typePart.first >= 0 && m_typePart.second >= 0);
    TU_ASSERT (m_keyPart.first >= 0 && m_keyPart.second >= 0);
}

//static const char *kValidChars =
//    "abcdefghijklmnopqrstuvwxyz"
//    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//    "0123456789";
//
//static bool
//is_valid_id(std::shared_ptr<const std::string> id)
//{
//    if (id->empty())
//        return false;
//    if (!std::isalpha(id->front()))
//        return false;
//    if (id->find_first_not_of(kValidChars, 1, id->size() - 1) != std::string::npos)
//        return false;
//    return true;
//}

bool
groove_model::PageId::isValid() const
{
    return !m_id->empty();
}

std::string
groove_model::PageId::getPrefix() const
{
    if (m_id->empty())
        return {};
    return m_id->substr(m_prefixPart.first, m_prefixPart.second);
}

std::string
groove_model::PageId::getKey() const
{
    if (m_id->empty())
        return {};
    return m_id->substr(m_keyPart.first, m_keyPart.second);
}

groove_data::CollationMode
groove_model::PageId::getCollation() const
{
    if (m_typePart.first < 0)
        return groove_data::CollationMode::COLLATION_UNKNOWN;
    char c = m_id->at(m_typePart.first);
    switch (c) {
        case 's':
            return groove_data::CollationMode::COLLATION_SORTED;
        case 'i':
            return groove_data::CollationMode::COLLATION_INDEXED;
        default:
            return groove_data::CollationMode::COLLATION_UNKNOWN;
    }
}

groove_data::DataKeyType
groove_model::PageId::getKeyType() const
{
    if (m_typePart.first < 0)
        return groove_data::DataKeyType::KEY_UNKNOWN;
    char c = m_id->at(m_typePart.first + 1);
    switch (c) {
        case 'c':
            return groove_data::DataKeyType::KEY_CATEGORY;
        case 'd':
            return groove_data::DataKeyType::KEY_DOUBLE;
        case 'i':
            return groove_data::DataKeyType::KEY_INT64;
        default:
            return groove_data::DataKeyType::KEY_UNKNOWN;
    }
}

groove_data::DataValueType
groove_model::PageId::getValueType() const
{
    if (m_typePart.first < 0)
        return groove_data::DataValueType::VALUE_TYPE_UNKNOWN;
    char c = m_id->at(m_typePart.first + 2);
    switch (c) {
        case 'd':
            return groove_data::DataValueType::VALUE_TYPE_DOUBLE;
        case 'i':
            return groove_data::DataValueType::VALUE_TYPE_INT64;
        case 's':
            return groove_data::DataValueType::VALUE_TYPE_STRING;
        default:
            return groove_data::DataValueType::VALUE_TYPE_UNKNOWN;
    }
}

std::string
groove_model::PageId::getBytes() const
{
    return *m_id;
}

bool
groove_model::PageId::operator<(const PageId &other) const
{
    return *m_id < *other.m_id;
}

bool
groove_model::PageId::operator<=(const PageId &other) const
{
    return *m_id <= *other.m_id;
}

bool
groove_model::PageId::operator>(const PageId &other) const
{
    return *m_id > *other.m_id;
}

bool
groove_model::PageId::operator>=(const PageId &other) const
{
    return *m_id >= *other.m_id;
}

bool
groove_model::PageId::operator==(const PageId &other) const
{
    return *m_id == *other.m_id;
}

bool
groove_model::PageId::operator!=(const PageId &other) const
{
    return *m_id != *other.m_id;
}

groove_model::PageId
groove_model::PageId::create(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataKeyType keyType,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    const std::string &keyBytes)
{
    TU_ASSERT (datasetUrl.isValid());
    TU_ASSERT (modelId != nullptr && !modelId->empty());
    TU_ASSERT (columnId != nullptr && !columnId->empty());

    std::string idBytes;
    std::pair<int,int> prefixPart;
    std::pair<int,int> typePart;
    std::pair<int,int> keyPart;

    absl::StrAppend(&idBytes, "page\x1f", datasetUrl.toString(), "\x1f", *modelId, "\x1f", *columnId);
    idBytes.push_back('\x1e');
    prefixPart.first = 0;
    prefixPart.second = idBytes.size();

    idBytes.push_back(collation_to_byte(collation));
    idBytes.push_back(key_type_to_byte(keyType));
    idBytes.push_back(value_type_to_byte(valueType));
    idBytes.push_back('\x1e');
    typePart.first = prefixPart.second;
    typePart.second = 4;

    absl::StrAppend(&idBytes, keyBytes);
    keyPart.first = typePart.first + typePart.second;
    keyPart.second = keyBytes.size();

    auto id = std::make_shared<const std::string>(idBytes);
    return PageId(id, prefixPart, typePart, keyPart);
}

groove_model::PageId
groove_model::PageId::create(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    Option<groove_data::Category> key)
{
    return create(datasetUrl, modelId, columnId,
        groove_data::DataKeyType::KEY_CATEGORY, valueType, collation,
        key_to_bytes(key));
}

groove_model::PageId
groove_model::PageId::create(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    Option<double> key)
{
    return create(datasetUrl, modelId, columnId,
        groove_data::DataKeyType::KEY_DOUBLE, valueType, collation,
        key_to_bytes(key));
}

groove_model::PageId
groove_model::PageId::create(
    const tempo_utils::Url &datasetUrl,
    std::shared_ptr<const std::string> modelId,
    std::shared_ptr<const std::string> columnId,
    groove_data::DataValueType valueType,
    groove_data::CollationMode collation,
    Option<tu_int64> key)
{
    return create(datasetUrl, modelId, columnId,
        groove_data::DataKeyType::KEY_INT64, valueType, collation,
        key_to_bytes(key));
}

groove_model::PageId
groove_model::PageId::fromString(std::string_view s)
{
    int endOfPrefixPart = s.find('\x1e');
    if (endOfPrefixPart < 0)
        return {};
    std::pair<int,int> prefixPart;
    prefixPart.first = 0;
    prefixPart.second = endOfPrefixPart;

    int endOfTypePart = s.find('\x1e', endOfPrefixPart + 1);
    if (endOfTypePart < 0)
        return {};
    if (endOfTypePart != endOfPrefixPart + 4)
        return {};
    std::pair<int,int> typePart;
    typePart.first = prefixPart.second;
    typePart.second = 4;

    std::pair<int,int> keyPart;
    keyPart.first = typePart.first + typePart.second;
    keyPart.second = s.size() - keyPart.first;

    return PageId(std::make_shared<const std::string>(s), prefixPart, typePart, keyPart);
}