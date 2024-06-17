
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/bar_stack_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::BarStackDatum::BarStackDatum()
    : key(), values()
{
}

groove_shapes::BarStackDatum::BarStackDatum(
    const groove_data::Category &key,
    const absl::flat_hash_map<std::string,std::pair<double,groove_data::DatumFidelity>> &values)
    : key(key), values(values)
{
}

groove_shapes::BarStackShape::BarStackShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::BarStackShape::configure(const SourceDescriptor &source)
{
    auto model = getModel(source.getDatasetUrl(), source.getModelId());
    std::vector<std::string> columnIds;
    if (!source.getColumnId().empty()) {
        columnIds.push_back(source.getColumnId());
    } else {
        for (auto iterator = model->columnsBegin(); iterator != model->columnsEnd(); iterator++) {
            columnIds.push_back(iterator->first);
        }
    }

    for (const auto &columnId : columnIds) {
        auto getModelResult = model->getIndexedColumn<groove_model::CategoryDouble>(columnId);
        if (getModelResult.isStatus())
            return getModelResult.getStatus();
        auto itemId = absl::StrCat(
            source.getDatasetUrl().toString(),
            " ", source.getModelId(),
            " ", columnId);
        m_columns[itemId] = getModelResult.getResult();
        m_items.insert(itemId);
    }

    return ShapesStatus::ok();
}

bool
groove_shapes::BarStackShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::BarStackShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_BAR_STACK;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarStackShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::BarStackShape::itemsEnd() const
{
    return m_items.cend();
}

groove_units::UnitDimension
groove_shapes::BarStackShape::getBarStackUnits()
{
//    UnitDimension valueDimension;
//
//    // if every source has valid units for value, and each source has the
//    // same units for value, then return the value unit dimensions.
//    // otherwise return empty dimension for value.
//    for (const auto &sourceId : listSourceIds()) {
//        auto source = getSource(sourceId);
//        auto descriptor = source.getStreamDescriptor();
//        auto column = source.getStreamColumn();
//        auto sourceValueDim = column->getUnitDimension();
//
//        if (valueDimension.isValid() && valueDimension != sourceValueDim) {
//            TU_LOG_INFO << this << " value dimension " << valueDimension.toString()
//                << " != " << sourceValueDim.toString();
//            return UnitDimension();
//        }
//        valueDimension = sourceValueDim;
//    }
//
//    return valueDimension;
    return groove_units::UnitDimension();
}

tempo_utils::Result<groove_shapes::BarStackDatum>
groove_shapes::BarStackShape::getBarStackValue(const groove_data::Category &key)
{
    BarStackDatum datum;
    datum.key = key;
    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto &itemId = iterator->first;
        auto column = iterator->second;
        auto result = column->getValue(key);
        if (result.isStatus())
            return result.getStatus();
        auto value = result.getResult();
        datum.values[itemId] = std::pair<double,groove_data::DatumFidelity>{value.value, value.fidelity};
    }
    return datum;
}

tempo_utils::Result<groove_shapes::BarStackDatumIterator>
groove_shapes::BarStackShape::getBarStackValues(
    const groove_data::CategoryRange &range)
{
    std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::CategoryDoubleColumnIterator,
                groove_data::CategoryDoubleDatum>>> inputs;

    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto itemId = iterator->first;
        auto column = iterator->second;
        auto result = column->getValues(range);
        if (result.isStatus())
            return result.getStatus();
        inputs.push_back({
            itemId,
            groove_iterator::PeekIterator<
                groove_model::CategoryDoubleColumnIterator,
                groove_data::CategoryDoubleDatum>(result.getResult())
        });
    }

    return BarStackDatumIterator(inputs);
}

groove_shapes::BarStackDatumIterator::BarStackDatumIterator()
{
}

groove_shapes::BarStackDatumIterator::BarStackDatumIterator(
    const std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::CategoryDoubleColumnIterator,groove_data::CategoryDoubleDatum>>> &inputs)
    : m_inputs(inputs)
{
}

bool
groove_shapes::BarStackDatumIterator::getNext(BarStackDatum &datum)
{
    groove_data::Category key;

    // determine which inputs are ready
    std::vector<int> ready;
    for (int i = 0; i < m_inputs.size(); i++) {
        auto &input = m_inputs.at(i).second;
        groove_data::CategoryDoubleDatum peek;
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
    absl::flat_hash_map<std::string,std::pair<double,groove_data::DatumFidelity>> values;
    for (const auto &i : ready) {
        auto &p = m_inputs.at(i);
        groove_data::CategoryDoubleDatum value;
        if (p.second.getNext(value)) {
            values[p.first] = std::pair<double,groove_data::DatumFidelity>{value.value, value.fidelity};
        }
    }
    datum = BarStackDatum(key, values);
    return true;
}
