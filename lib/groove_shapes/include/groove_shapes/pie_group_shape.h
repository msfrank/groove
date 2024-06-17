#ifndef GROOVE_SHAPES_PIE_GROUP_SHAPE_H
#define GROOVE_SHAPES_PIE_GROUP_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_iterator/peek_iterator_template.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shapes_result.h"

namespace groove_shapes {

    struct PieGroupDatum {
        std::string id;
        absl::flat_hash_map<groove_data::Category, std::pair<double,groove_data::DatumFidelity>> values;
        double total;

        PieGroupDatum();
        PieGroupDatum(
            const std::string &id,
            const absl::flat_hash_map<groove_data::Category, std::pair<double,groove_data::DatumFidelity>> &values,
            double total);
    };

    class PieGroupShape : public BaseShape {

    public:
        PieGroupShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);

        tempo_utils::Status configure(const SourceDescriptor &source) override;

        bool isValid() const;

        DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

        tempo_utils::Result<PieGroupDatum> getPieGroupValue(const std::string &itemId);
        tempo_utils::Result<PieGroupDatum> getPieGroupValue(
            const std::string &itemId,
            const groove_data::CategoryRange &range);

    private:
        absl::flat_hash_set<std::string> m_items;
        absl::flat_hash_map<
            std::string,
            std::shared_ptr<groove_model::IndexedColumn<groove_model::CategoryDouble>>> m_columns;
    };
}

#endif // GROOVE_SHAPES_PIE_GROUP_SHAPE_H