#ifndef GROOVE_SHAPES_BAR_ANNOTATION_SHAPE_H
#define GROOVE_SHAPES_BAR_ANNOTATION_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct BarAnnotationDatum {
        groove_data::Category key;
        std::string value;
        groove_data::DatumFidelity fidelity;

        BarAnnotationDatum();
        BarAnnotationDatum(
            const groove_data::Category &key,
            const std::string &value,
            groove_data::DatumFidelity fidelity);
    };

    class BarAnnotationDatumIterator : public Iterator<BarAnnotationDatum> {
    public:
        BarAnnotationDatumIterator();
        BarAnnotationDatumIterator(groove_model::CategoryStringColumnIterator input);
        bool getNext(BarAnnotationDatum &datum) override;
    private:
        groove_model::CategoryStringColumnIterator m_input;
    };

    class BarAnnotationShape : public BaseShape {

    public:
        BarAnnotationShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        tempo_utils::Result<BarAnnotationDatum> getBarAnnotationValue(
            const std::string &itemId,
            const groove_data::Category &key);

        tempo_utils::Result<BarAnnotationDatumIterator> getBarAnnotationValues(
            const std::string &itemId,
            const groove_data::CategoryRange &range);

        std::vector<BarAnnotationDatum> describeItemsAt(
            const groove_data::Category &fromKeyInclusive,
            const groove_data::Category &toKeyInclusive,
            const std::vector<std::string> &itemIds = {});

//    signals:
//        void barCountChanged();
//        void barPropertiesChanged();

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryString>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_BAR_ANNOTATION_SHAPE_H