
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_model/page_traits.h>
#include <groove_shapes/series_stack_shape.h>
#include <tempo_utils/logging.h>

groove_shapes::SeriesStackDatum::SeriesStackDatum()
    : key{}, values()
{
}

groove_shapes::SeriesStackDatum::SeriesStackDatum(
    double key,
    const absl::flat_hash_map<std::string, std::pair<double,groove_data::DatumFidelity>> &values)
    : key(key), values(values)
{
}

groove_shapes::SeriesStackShape::SeriesStackShape(
    std::shared_ptr<groove_model::GrooveDatabase> database,
    const std::string &shapeId)
    : BaseShape(database, shapeId)
{
}

tempo_utils::Status
groove_shapes::SeriesStackShape::configure(const SourceDescriptor &source)
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
        auto getModelResult = model->getIndexedColumn<groove_model::DoubleDouble>(columnId);
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
groove_shapes::SeriesStackShape::isValid() const
{
    return true;
}

groove_shapes::DataShapeType
groove_shapes::SeriesStackShape::getShapeType() const
{
    return groove_shapes::DataShapeType::SHAPE_SERIES_STACK;
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesStackShape::itemsBegin() const
{
    return m_items.cbegin();
}

absl::flat_hash_set<std::string>::const_iterator
groove_shapes::SeriesStackShape::itemsEnd() const
{
    return m_items.cend();
}

//QPair<UnitDimension,UnitDimension>
//groove_shapes::SeriesStackShape::getSeriesStackUnits()
//{
//    UnitDimension indexDimension;
//    UnitDimension valueDimension;
//
//    // if every source has valid units for index and value, and each source has the
//    // same units for index and value, then return the index and value unit dimensions.
//    // otherwise return empty dimension for index and value.
//    for (const auto &sourceId : listSourceIds()) {
//        auto source = getSource(sourceId);
//        auto descriptor = source.getStreamDescriptor();
//        auto column = source.getStreamColumn();
//        auto sourceIndexDim = descriptor->getIndexUnitDimension();
//        auto sourceValueDim = column->getUnitDimension();
//
//        if (indexDimension.isValid() && indexDimension != sourceIndexDim)
//            return {UnitDimension(), UnitDimension()};
//        indexDimension = sourceIndexDim;
//
//        if (valueDimension.isValid() && valueDimension != sourceValueDim)
//            return {UnitDimension(), UnitDimension()};
//        valueDimension = sourceValueDim;
//    }
//
//    return {indexDimension, valueDimension};
//}

tempo_utils::Result<groove_shapes::SeriesStackDatum>
groove_shapes::SeriesStackShape::getSeriesStackValue(double key)
{
    SeriesStackDatum datum;
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

tempo_utils::Result<groove_shapes::SeriesStackDatumIterator>
groove_shapes::SeriesStackShape::getSeriesStackValues(const groove_data::DoubleRange &range)
{
    std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::DoubleDoubleColumnIterator,
                groove_data::DoubleDoubleDatum>>> inputs;

    for (auto iterator = m_columns.cbegin(); iterator != m_columns.cend(); iterator++) {
        auto itemId = iterator->first;
        auto column = iterator->second;
        auto result = column->getValues(range);
        if (result.isStatus())
            return result.getStatus();
        inputs.push_back({
            itemId,
            groove_iterator::PeekIterator<
               groove_model::DoubleDoubleColumnIterator,
               groove_data::DoubleDoubleDatum>(result.getResult())
        });
    }

    return SeriesStackDatumIterator(inputs);
}

//QStringList
//groove_shapes::SeriesStackShape::describeItemsAt(qreal key, qreal value, qreal hitradius)
//{
//    return QStringList();
//}

groove_shapes::SeriesStackDatumIterator::SeriesStackDatumIterator()
{
}

groove_shapes::SeriesStackDatumIterator::SeriesStackDatumIterator(
    const std::vector<
        std::pair<
            std::string,
            groove_iterator::PeekIterator<
                groove_model::DoubleDoubleColumnIterator,
                groove_data::DoubleDoubleDatum>>> &inputs)
    : m_inputs(inputs)
{
}

bool
groove_shapes::SeriesStackDatumIterator::getNext(SeriesStackDatum &datum)
{
    double key;

    // determine which inputs are ready
    std::vector<int> ready;
    for (int i = 0; i < m_inputs.size(); i++) {
        auto &input = m_inputs.at(i).second;
        groove_data::DoubleDoubleDatum peek;
        if (input.peekNext(peek)) {
            if (i == 0 || peek.key < key) {
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
        groove_data::DoubleDoubleDatum value;
        if (p.second.getNext(value)) {
            values[p.first] = std::pair<double,groove_data::DatumFidelity>{value.value, value.fidelity};
        }
    }
    datum = SeriesStackDatum(key, values);
    return true;
}
