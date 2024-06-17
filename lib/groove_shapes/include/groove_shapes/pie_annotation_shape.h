#ifndef GROOVE_SHAPES_PIE_ANNOTATION_SHAPE_H
#define GROOVE_SHAPES_PIE_ANNOTATION_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct PieAnnotationDatum {
        groove_data::Category key;
        std::string value;
        groove_data::DatumFidelity fidelity;

        PieAnnotationDatum();
        PieAnnotationDatum(
            const groove_data::Category &key,
            const std::string &value,
            groove_data::DatumFidelity fidelity);
    };

    class PieAnnotationDatumIterator : public Iterator<PieAnnotationDatum> {
    public:
        PieAnnotationDatumIterator();
        PieAnnotationDatumIterator(groove_model::CategoryStringColumnIterator input);
        bool getNext(PieAnnotationDatum &datum) override;
    private:
        groove_model::CategoryStringColumnIterator m_input;
    };

    class PieAnnotationShape : public BaseShape {

    public:
        PieAnnotationShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        groove_shapes::DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        tempo_utils::Result<PieAnnotationDatum> getPieAnnotationValue(
            const std::string &itemId,
            const groove_data::Category &key);

        tempo_utils::Result<PieAnnotationDatumIterator> getPieAnnotationValues(
            const std::string &itemId,
            const groove_data::CategoryRange &range);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryString>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_PIE_ANNOTATION_SHAPE_H