
#include <groove_model/generated/schema.h>
#include <groove_model/schema_attr.h>
#include <groove_model/schema_column.h>
#include <groove_model/schema_model.h>
#include <groove_model/schema_namespace.h>
#include <groove_model/schema_state.h>
#include <tempo_utils/memory_bytes.h>

groove_model::SchemaState::SchemaState()
{
}

groove_model::SchemaState::~SchemaState()
{
    for (auto *schemaNamspace : m_schemaNamespaces) {
        delete schemaNamspace;
    }
    for (auto *schemaAttr : m_schemaAttrs) {
        delete schemaAttr;
    }
    for (auto *schemaColumn : m_schemaColumns) {
        delete schemaColumn;
    }
    for (auto *schemaModel : m_schemaModels) {
        delete schemaModel;
    }
}

bool
groove_model::SchemaState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return getNamespace(nsUrl) != nullptr;
}

groove_model::SchemaNamespace *
groove_model::SchemaState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_schemaNamespaces.size()))
        return m_schemaNamespaces.at(index);
    return nullptr;
}

groove_model::SchemaNamespace *
groove_model::SchemaState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (m_namespaceIndex.contains(nsUrl))
        return m_namespaceIndex.at(nsUrl);
    return nullptr;
}

tempo_utils::Result<groove_model::SchemaNamespace *>
groove_model::SchemaState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl))
        return m_namespaceIndex.at(nsUrl);

    NamespaceAddress address(m_schemaNamespaces.size());
    auto *schemaNamespace = new SchemaNamespace(nsUrl, address, this);
    m_schemaNamespaces.push_back(schemaNamespace);
    m_namespaceIndex[nsUrl] = schemaNamespace;
    return schemaNamespace;
}

tempo_utils::Result<tu_uint32>
groove_model::SchemaState::putNamespace(const char *nsString)
{
    auto nsUrl = tempo_utils::Url::fromString(nsString);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto *archetypeNs = putNamespaceResult.getResult();
    return archetypeNs->getAddress().getAddress();
}

std::vector<groove_model::SchemaNamespace *>::const_iterator
groove_model::SchemaState::namespacesBegin() const
{
    return m_schemaNamespaces.cbegin();
}

std::vector<groove_model::SchemaNamespace *>::const_iterator
groove_model::SchemaState::namespacesEnd() const
{
    return m_schemaNamespaces.cend();
}

int
groove_model::SchemaState::numNamespaces() const
{
    return m_schemaNamespaces.size();
}

tempo_utils::Result<groove_model::SchemaAttr *>
groove_model::SchemaState::appendAttr(AttrId id, tempo_utils::AttrValue value)
{
    AttrAddress address(m_schemaAttrs.size());
    auto *attr = new SchemaAttr(id, value, address, this);
    m_schemaAttrs.push_back(attr);
    return attr;
}

groove_model::SchemaAttr *
groove_model::SchemaState::getAttr(int index) const
{
    if (0 <= index && std::cmp_less(index, m_schemaAttrs.size()))
        return m_schemaAttrs.at(index);
    return nullptr;
}

std::vector<groove_model::SchemaAttr *>::const_iterator
groove_model::SchemaState::attrsBegin() const
{
    return m_schemaAttrs.cbegin();
}

std::vector<groove_model::SchemaAttr *>::const_iterator
groove_model::SchemaState::attrsEnd() const
{
    return m_schemaAttrs.cend();
}

int
groove_model::SchemaState::numAttrs() const
{
    return m_schemaAttrs.size();
}

tempo_utils::Result<groove_model::SchemaModel *>
groove_model::SchemaState::putModel(
    std::string_view modelId,
    ModelKeyType keyType,
    ModelKeyCollation keyCollation)
{
    if (m_modelIndex.contains(modelId))
        return ModelStatus::forCondition(ModelCondition::kModelInvariant,
            "schema contains duplicate model {}", modelId);
    ModelAddress address(m_schemaModels.size());
    auto *schemaModel = new SchemaModel(std::string(modelId), keyType, keyCollation, address, this);
    m_schemaModels.push_back(schemaModel);
    m_modelIndex[modelId] = schemaModel;
    return schemaModel;
}

groove_model::SchemaModel *
groove_model::SchemaState::getModel(int index) const
{
    if (0 <= index && std::cmp_less(index, m_schemaModels.size()))
        return m_schemaModels.at(index);
    return nullptr;
}

std::vector<groove_model::SchemaModel *>::const_iterator
groove_model::SchemaState::modelsBegin() const
{
    return m_schemaModels.cbegin();
}

std::vector<groove_model::SchemaModel *>::const_iterator
groove_model::SchemaState::modelsEnd() const
{
    return m_schemaModels.cend();
}

int
groove_model::SchemaState::numModels() const
{
    return m_schemaModels.size();
}

tempo_utils::Result<groove_model::SchemaColumn *>
groove_model::SchemaState::appendColumn(
    std::string_view columnId,
    ColumnValueType valueType,
    ColumnValueFidelity valueFidelity)
{
    ColumnAddress address(m_schemaColumns.size());
    auto *schemaColumn = new SchemaColumn(std::string(columnId), valueType, valueFidelity, address, this);
    m_schemaColumns.push_back(schemaColumn);
    return schemaColumn;
}

groove_model::SchemaColumn *
groove_model::SchemaState::getColumn(int index) const
{
    if (0 <= index && std::cmp_less(index, m_schemaColumns.size()))
        return m_schemaColumns.at(index);
    return nullptr;
}

std::vector<groove_model::SchemaColumn *>::const_iterator
groove_model::SchemaState::columnsBegin() const
{
    return m_schemaColumns.cbegin();
}

std::vector<groove_model::SchemaColumn *>::const_iterator
groove_model::SchemaState::columnsEnd() const
{
    return m_schemaColumns.cend();
}

int
groove_model::SchemaState::numColumns() const
{
    return m_schemaModels.size();
}

static std::pair<gms1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_utils::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_utils::ValueType::Nil: {
            auto type = gms1::Value::TrueFalseNilValue;
            auto offset = gms1::CreateTrueFalseNilValue(buffer, gms1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Bool: {
            auto type = gms1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? gms1::TrueFalseNil::True : gms1::TrueFalseNil::False;
            auto offset = gms1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Int64: {
            auto type = gms1::Value::Int64Value;
            auto offset = gms1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Float64: {
            auto type = gms1::Value::Float64Value;
            auto offset = gms1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt64: {
            auto type = gms1::Value::UInt64Value;
            auto offset = gms1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt32: {
            auto type = gms1::Value::UInt32Value;
            auto offset = gms1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt16: {
            auto type = gms1::Value::UInt16Value;
            auto offset = gms1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt8: {
            auto type = gms1::Value::UInt8Value;
            auto offset = gms1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::String: {
            auto type = gms1::Value::StringValue;
            auto offset = gms1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<groove_model::GrooveSchema>
groove_model::SchemaState::toSchema(bool noIdentifier) const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<gms1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<gms1::AttrDescriptor>> attrs_vector;
    std::vector<flatbuffers::Offset<gms1::ColumnDescriptor>> columns_vector;
    std::vector<flatbuffers::Offset<gms1::ModelDescriptor>> models_vector;

    // serialize namespaces
    for (const auto *schemaNamespace : m_schemaNamespaces) {
        auto fb_nsUrl = buffer.CreateString(schemaNamespace->getNsUrl().toString());
        namespaces_vector.push_back(gms1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize attrs
    for (const auto *schemaAttr : m_schemaAttrs) {
        auto id = schemaAttr->getAttrId();
        auto value = schemaAttr->getAttrValue();
        auto p = serialize_value(buffer, value);

        attrs_vector.push_back(gms1::CreateAttrDescriptor(buffer,
            id.getAddress().getAddress(), id.getType(), p.first, p.second));
    }
    auto fb_attrs = buffer.CreateVector(attrs_vector);

    // serialize columns
    for (const auto *schemaColumn : m_schemaColumns) {
        auto columnId = buffer.CreateString(schemaColumn->getColumnId());

        gms1::ValueType valueType;
        switch (schemaColumn->getValueType()) {
            case ColumnValueType::Double:
                valueType = gms1::ValueType::Double;
                break;
            case ColumnValueType::Int64:
                valueType = gms1::ValueType::Int64;
                break;
            case ColumnValueType::String:
                valueType = gms1::ValueType::String;
                break;
            default:
                return ModelStatus::forCondition(ModelCondition::kModelInvariant,
                    "invalid value type for column {}", schemaColumn->getColumnId());
        }

        gms1::ValueFidelity valueFidelity;
        switch (schemaColumn->getValueFidelity()) {
            case ColumnValueFidelity::OnlyValidValue:
                valueFidelity = gms1::ValueFidelity::OnlyValidValue;
                break;
            case ColumnValueFidelity::OnlyValidOrEmpty:
                valueFidelity = gms1::ValueFidelity::OnlyValidOrEmpty;
                break;
            case ColumnValueFidelity::AnyFidelityAllowed:
                valueFidelity = gms1::ValueFidelity::AnyFidelityAllowed;
                break;
            default:
                return ModelStatus::forCondition(ModelCondition::kModelInvariant,
                    "invalid value fidelity for column {}", schemaColumn->getColumnId());
        }

        std::vector<tu_uint32> attrs;
        for (auto it = schemaColumn->attrsBegin(); it != schemaColumn->attrsEnd(); it++) {
            auto address = it->second;
            TU_ASSERT (address.isValid());
            attrs.push_back(address.getAddress());
        }

        columns_vector.push_back(gms1::CreateColumnDescriptor(buffer,
            columnId, buffer.CreateVector(attrs), valueType, valueFidelity));
    }
    auto fb_columns = buffer.CreateVector(columns_vector);

    // serialize models
    for (const auto *schemaModel : m_schemaModels) {
        auto modelId = buffer.CreateString(schemaModel->getModelId());

        gms1::KeyType keyType;
        switch (schemaModel->getKeyType()) {
            case ModelKeyType::Category:
                keyType = gms1::KeyType::Category;
                break;
            case ModelKeyType::Double:
                keyType = gms1::KeyType::Double;
                break;
            case ModelKeyType::Int64:
                keyType = gms1::KeyType::Int64;
                break;
            default:
                return ModelStatus::forCondition(ModelCondition::kModelInvariant,
                    "invalid key type for model {}", schemaModel->getModelId());
        }

        gms1::KeyCollation keyCollation;
        switch (schemaModel->getKeyCollation()) {
            case ModelKeyCollation::Sorted:
                keyCollation = gms1::KeyCollation::Sorted;
                break;
            case ModelKeyCollation::Indexed:
                keyCollation = gms1::KeyCollation::Indexed;
                break;
            default:
                return ModelStatus::forCondition(ModelCondition::kModelInvariant,
                    "invalid key collation for model {}", schemaModel->getModelId());
        }

        std::vector<tu_uint32> attrs;
        for (auto it = schemaModel->attrsBegin(); it != schemaModel->attrsEnd(); it++) {
            auto address = it->second;
            TU_ASSERT (address.isValid());
            attrs.push_back(address.getAddress());
        }

        std::vector<tu_uint32> columns;
        for (auto it = schemaModel->columnsBegin(); it != schemaModel->columnsEnd(); it++) {
            auto &address = *it;
            TU_ASSERT (address.isValid());
            columns.push_back(address.getAddress());
        }

        models_vector.push_back(gms1::CreateModelDescriptor(buffer,
            modelId, buffer.CreateVector(attrs), keyType, keyCollation, buffer.CreateVector(columns)));
    }
    auto fb_models = buffer.CreateVectorOfSortedTables(&models_vector);

    // build schema from buffer
    gms1::SchemaBuilder schemaBuilder(buffer);

    schemaBuilder.add_abi(gms1::SchemaVersion::Version1);
    schemaBuilder.add_namespaces(fb_namespaces);
    schemaBuilder.add_attrs(fb_attrs);
    schemaBuilder.add_columns(fb_columns);
    schemaBuilder.add_models(fb_models);

    // serialize schema and mark the buffer as finished
    auto schema = schemaBuilder.Finish();
    if (noIdentifier) {
        buffer.Finish(schema);
    } else {
        buffer.Finish(schema, gms1::SchemaIdentifier());
    }

    // copy the flatbuffer into our own byte array and instantiate schema
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return GrooveSchema(bytes);
}