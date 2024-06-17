#ifndef SERIES_ANNOTATION_SHAPE_H
#define SERIES_ANNOTATION_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_units/unit_dimension.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct SeriesAnnotationDatum {
        double key;
        std::string value;
        groove_data::DatumFidelity fidelity;

        SeriesAnnotationDatum();
        SeriesAnnotationDatum(double key, const std::string &value, groove_data::DatumFidelity fidelity);
    };

    class SeriesAnnotationDatumIterator : public Iterator<SeriesAnnotationDatum> {
    public:
        SeriesAnnotationDatumIterator();
        SeriesAnnotationDatumIterator(groove_model::DoubleStringColumnIterator input);
        bool getNext(SeriesAnnotationDatum &datum) override;
    private:
        groove_model::DoubleStringColumnIterator m_input;
    };

    class SeriesAnnotationShape : public BaseShape {

    public:
        SeriesAnnotationShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        groove_shapes::DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        groove_units::UnitDimension getSeriesAnnotationUnits(const std::string &itemId);

        tempo_utils::Result<SeriesAnnotationDatum> getSeriesAnnotationValue(
            const std::string &itemId,
            double key);

        tempo_utils::Result<SeriesAnnotationDatumIterator> getSeriesAnnotationValues(
            const std::string &itemId,
            const groove_data::DoubleRange &range);

        std::vector<SeriesAnnotationDatum> describeItemsAt(
            double fromKeyInclusive,
            double toKeyInclusive,
            const std::vector<std::string> &itemIds = {});

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::DoubleString>>> m_columns;
    };
}

#endif // SERIES_ANNOTATION_SHAPE_H
