#ifndef GROOVE_SHAPES_BAR_STACK_SHAPE_H
#define GROOVE_SHAPES_BAR_STACK_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_iterator/peek_iterator_template.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_units/unit_dimension.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct BarStackDatum {
        groove_data::Category key;
        absl::flat_hash_map<std::string, std::pair<double,groove_data::DatumFidelity>> values;

        BarStackDatum();
        BarStackDatum(
            const groove_data::Category &key,
            const absl::flat_hash_map<std::string, std::pair<double,groove_data::DatumFidelity>> &values);
    };

    class BarStackDatumIterator : public Iterator<BarStackDatum> {
    public:
        BarStackDatumIterator();
        BarStackDatumIterator(
            const std::vector<
                std::pair<
                    std::string,
                    groove_iterator::PeekIterator<
                        groove_model::CategoryDoubleColumnIterator,groove_data::CategoryDoubleDatum>>> &inputs);
        bool getNext(BarStackDatum &datum) override;
    private:
        std::vector<
            std::pair<
                std::string,
                groove_iterator::PeekIterator<
                    groove_model::CategoryDoubleColumnIterator,groove_data::CategoryDoubleDatum>>> m_inputs;
    };

    class BarStackShape : public BaseShape {

    public:
        BarStackShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        groove_units::UnitDimension getBarStackUnits();

        tempo_utils::Result<BarStackDatum> getBarStackValue(const groove_data::Category &key);

        tempo_utils::Result<BarStackDatumIterator> getBarStackValues(
            const groove_data::CategoryRange &range);

        //QStringList describeItemsAt(double key, double value, double hitradius);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryDouble>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_BAR_STACK_SHAPE_H