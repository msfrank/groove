#ifndef GROOVE_SHAPES_BAR_GROUP_SHAPE_H
#define GROOVE_SHAPES_BAR_GROUP_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <groove_units/unit_dimension.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct BarGroupDatum {
        groove_data::Category key;
        double value;
        groove_data::DatumFidelity fidelity;

        BarGroupDatum();
        BarGroupDatum(const groove_data::Category &key, double value, groove_data::DatumFidelity fidelity);
    };

    class BarGroupDatumIterator : public Iterator<BarGroupDatum> {
    public:
        BarGroupDatumIterator();
        BarGroupDatumIterator(groove_model::CategoryDoubleColumnIterator input);
        bool getNext(BarGroupDatum &datum) override;
    private:
        groove_model::CategoryDoubleColumnIterator m_input;
    };

    class BarGroupShape : public BaseShape {

    public:
        BarGroupShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        groove_units::UnitDimension getBarGroupUnits(const std::string &itemId);

        tempo_utils::Result<BarGroupDatum> getBarGroupValue(
            const std::string &itemId,
            const groove_data::Category &key);

        tempo_utils::Result<BarGroupDatumIterator> getBarGroupValues(
            const std::string &itemId,
            const groove_data::CategoryRange &range);

        std::vector<BarGroupDatum> describeItemsAt(
            const groove_data::Category &key,
            double value,
            double hitradius);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryDouble>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_BAR_GROUP_SHAPE_H