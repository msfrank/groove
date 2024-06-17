#ifndef GROOVE_SHAPES_SERIES_GROUP_SHAPE_H
#define GROOVE_SHAPES_SERIES_GROUP_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct SeriesGroupDatum {
        double key;
        double value;
        groove_data::DatumFidelity fidelity;

        SeriesGroupDatum();

        SeriesGroupDatum(double key, double value, groove_data::DatumFidelity fidelity);
    };

    class SeriesGroupDatumIterator : public Iterator<SeriesGroupDatum> {
    public:
        SeriesGroupDatumIterator();
        SeriesGroupDatumIterator(groove_model::DoubleDoubleColumnIterator input);
        bool getNext(SeriesGroupDatum &datum) override;
    private:
        groove_model::DoubleDoubleColumnIterator m_input;
    };

    class SeriesGroupShape : public BaseShape {

    public:
        SeriesGroupShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        tempo_utils::Result<SeriesGroupDatum> getSeriesGroupValue(
            const std::string &seriesId,
            double key);

        tempo_utils::Result<SeriesGroupDatumIterator> getSeriesGroupValues(
            const std::string &seriesId,
            const groove_data::DoubleRange &range);

        std::vector<SeriesGroupDatum> describeItemsAt(
            double fromKeyInclusive,
            double toKeyInclusive,
            const std::vector<std::string> &seriesIds = {});

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleDouble>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_SERIES_GROUP_SHAPE_H