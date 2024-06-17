
#include <groove_model/column_traits.h>
#include <groove_model/indexed_variant_column.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/event_table_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::EventTableDatum::EventTableDatum()
    : key(), values()
{
}

groove_shapes::EventTableDatum::EventTableDatum(
    const groove_data::Category &key,
    const absl::flat_hash_map<
        std::string,
        std::pair<groove_model::VariantValue,groove_data::DatumFidelity>> &values)
    : key(key), values(values)
{
}

groove_shapes::EventTableShape::EventTableShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::EventTableShape::configure(const SourceDescriptor &source)
{
    auto model = getModel(source.getDatasetUrl(), source.getModelId());
    absl::flat_hash_map<std::string,groove_model::ColumnDef> columnDefs;
    if (!source.getColumnId().empty()) {
        auto columnId = source.getColumnId();
        auto def = model->getColumnDef(columnId);
        columnDefs[columnId] = groove_model::ColumnDef(
            groove_data::CollationMode::COLLATION_INDEXED,
            groove_data::DataKeyType::KEY_CATEGORY,
            def.getValue());
    } else {
        for (auto iterator = model->columnsBegin(); iterator != model->columnsEnd(); iterator++) {
            columnDefs[iterator->first] = groove_model::ColumnDef(
                groove_data::CollationMode::COLLATION_INDEXED,
                groove_data::DataKeyType::KEY_CATEGORY,
                iterator->second.getValue());
        }
    }

    for (auto iterator = columnDefs.cbegin(); iterator != columnDefs.cend(); iterator++) {
        auto columnId = iterator->first;
        auto columnDef = iterator->second;

        std::shared_ptr<
            groove_model::AbstractIndexedVariantColumn<
                groove_data::Category,groove_data::CategoryRange>> column;
        switch (columnDef.getValue()) {
            case groove_data::DataValueType::VALUE_TYPE_DOUBLE: {
                auto getColumnResult = model->getIndexedColumn<groove_model::CategoryDouble>(columnId);
                if (getColumnResult.isStatus())
                    return getColumnResult.getStatus();
                auto indexedColumn = getColumnResult.getResult();
                column = groove_model::IndexedVariantColumn<groove_model::CategoryDouble>::create(indexedColumn);
                break;
            }
            case groove_data::DataValueType::VALUE_TYPE_INT64: {
                auto getColumnResult = model->getIndexedColumn<groove_model::CategoryInt64>(columnId);
                if (getColumnResult.isStatus())
                    return getColumnResult.getStatus();
                auto indexedColumn = getColumnResult.getResult();
                column = groove_model::IndexedVariantColumn<groove_model::CategoryInt64>::create(indexedColumn);
                break;
            }
            case groove_data::DataValueType::VALUE_TYPE_STRING: {
                auto getColumnResult = model->getIndexedColumn<groove_model::CategoryString>(columnId);
                if (getColumnResult.isStatus())
                    return getColumnResult.getStatus();
                auto indexedColumn = getColumnResult.getResult();
                column = groove_model::IndexedVariantColumn<groove_model::CategoryString>::create(indexedColumn);
                break;
            }
            default:
                return ShapesStatus::forCondition(ShapesCondition::kInvalidSource, "unknown column def value type");
        }

        auto itemId = absl::StrCat(
            source.getDatasetUrl().toString(),
            " ", source.getModelId(),
            " ", columnId);
        m_columns[itemId] = column;
        m_items.insert(itemId);
    }

    return ShapesStatus::ok();
}

bool
groove_shapes::EventTableShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::EventTableShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_EVENT_TABLE;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::EventTableShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::EventTableShape::itemsEnd() const
{
    return m_items.cend();
}

groove_data::DataValueType
groove_shapes::EventTableShape::getEventTableColumnValueType(const std::string &itemId) const
{
    if (!m_columns.contains(itemId))
        return groove_data::DataValueType::VALUE_TYPE_UNKNOWN;
    auto column = m_columns.at(itemId);
    return column->getValueType();
}

tempo_utils::Result<groove_shapes::EventTableDatum>
groove_shapes::EventTableShape::getEventTableValue(const groove_data::Category &key)
{
    EventTableDatum datum;
    datum.key = key;
    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto &itemId = iterator->first;
        auto column = iterator->second;
        auto result = column->getValue(key);
        if (result.isStatus())
            return result.getStatus();
        auto value = result.getResult();
        datum.values[itemId] = std::pair<groove_model::VariantValue,groove_data::DatumFidelity>{value.value, value.fidelity};
    }
    return datum;
}

tempo_utils::Result<groove_shapes::EventTableDatumIterator>
groove_shapes::EventTableShape::getEventTableValues(const groove_data::CategoryRange &range)
{
    std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::VariantValueDatumIterator<groove_data::Category>,
                groove_model::VariantValueDatum<groove_data::Category>>>> inputs;

    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto itemId = iterator->first;
        auto column = iterator->second;
        auto result = column->getValues(range);
        if (result.isStatus())
            return result.getStatus();
        inputs.push_back({
            itemId,
            groove_iterator::PeekIterator<
                groove_model::VariantValueDatumIterator<groove_data::Category>,
                groove_model::VariantValueDatum<groove_data::Category>>(result.getResult())
            });
    }

    return EventTableDatumIterator(inputs);
}

groove_shapes::EventTableDatumIterator::EventTableDatumIterator()
{
}

groove_shapes::EventTableDatumIterator::EventTableDatumIterator(
    const std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::VariantValueDatumIterator<groove_data::Category>,
                groove_model::VariantValueDatum<groove_data::Category>>>> &inputs)
    : m_inputs(inputs)
{
}

bool
groove_shapes::EventTableDatumIterator::getNext(EventTableDatum &datum)
{
    groove_data::Category key;

    // determine which inputs are ready
    std::vector<int> ready;
    for (int i = 0; i < m_inputs.size(); i++) {
        auto &input = m_inputs.at(i).second;
        groove_model::VariantValueDatum<groove_data::Category> peek;
        if (input.peekNext(peek)) {
            if (key.isEmpty() || peek.key < key) {
                ready.clear();
                ready.push_back(i);
                key = peek.key;
            } else if (peek.key == key) {
                ready.push_back(i);
            }
        }
    }

    // construct the datum
    absl::flat_hash_map<
        std::string,
        std::pair<
            groove_model::VariantValue,
            groove_data::DatumFidelity>> values;
    for (const auto &i : ready) {
        auto &p = m_inputs.at(i);
        groove_model::VariantValueDatum<groove_data::Category> value;
        if (p.second.getNext(value)) {
            values[p.first] = std::pair<
                groove_model::VariantValue,
                groove_data::DatumFidelity>{value.value, value.fidelity};
        }
    }
    datum = EventTableDatum(key, values);
    return true;
}
