#ifndef SERIES_STACK_SHAPE_H
#define SERIES_STACK_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_iterator/peek_iterator_template.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct SeriesStackDatum {
        double key;
        absl::flat_hash_map<std::string, std::pair<double,groove_data::DatumFidelity>> values;

        SeriesStackDatum();
        SeriesStackDatum(
            double key,
            const absl::flat_hash_map<std::string, std::pair<double,groove_data::DatumFidelity>> &values);
    };

    class SeriesStackDatumIterator : public Iterator<SeriesStackDatum> {
    public:
        SeriesStackDatumIterator();
        SeriesStackDatumIterator(
            const std::vector<
                std::pair<
                    std::string,
                    groove_iterator::PeekIterator<
                        groove_model::DoubleDoubleColumnIterator,
                        groove_data::DoubleDoubleDatum>>> &inputs);
        bool getNext(SeriesStackDatum &datum) override;
    private:
        std::vector<
            std::pair<
                std::string,
                groove_iterator::PeekIterator<
                    groove_model::DoubleDoubleColumnIterator,
                    groove_data::DoubleDoubleDatum>>> m_inputs;
    };

    class SeriesStackShape : public BaseShape {

    public:
        SeriesStackShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        //QPair<UnitDimension, UnitDimension> getSeriesStackUnits();

        tempo_utils::Result<SeriesStackDatum> getSeriesStackValue(double key);

        tempo_utils::Result<SeriesStackDatumIterator> getSeriesStackValues(
            const groove_data::DoubleRange &range);

        //QStringList describeItemsAt(qreal key, qreal value, qreal hitradius);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleDouble>>> m_columns;
    };
}

#endif // SERIES_STACK_SHAPE_H