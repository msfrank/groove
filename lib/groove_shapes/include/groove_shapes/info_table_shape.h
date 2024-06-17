#ifndef INFO_TABLE_SHAPE_H
#define INFO_TABLE_SHAPE_H

#include <absl/container/flat_hash_map.h>

#include <groove_data/data_types.h>
#include <groove_model/column_traits.h>
#include <groove_model/model_types.h>
#include <tempo_units/unit_dimension.h>
#include <tempo_utils/iterator_template.h>

#include "base_shape.h"
#include "shape_types.h"

namespace groove_shapes {

    class InfoTableShape : public BaseShape {

    public:
        InfoTableShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId,
            QObject *parent = Q_NULLPTR);

        bool isValid() const;

        groove_shapes::DataShapeType getShapeType() const override;

        absl::flat_hash_set<std::string>::const_iterator itemsBegin() const;
        absl::flat_hash_set<std::string>::const_iterator itemsEnd() const;

    };
}

#endif // INFO_TABLE_SHAPE_H
