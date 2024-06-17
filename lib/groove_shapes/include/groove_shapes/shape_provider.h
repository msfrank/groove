#ifndef GROOVE_SHAPES_SHAPE_PROVIDER_H
#define GROOVE_SHAPES_SHAPE_PROVIDER_H

#include <groove_model/groove_database.h>

#include "shapes_result.h"
#include "shape_types.h"

namespace groove_shapes {

    class ShapeProvider {

    public:
        ShapeProvider(std::shared_ptr<groove_model::GrooveDatabase> db);

        std::shared_ptr<groove_model::GrooveDatabase> getDatabase() const;

    private:
        std::shared_ptr<groove_model::GrooveDatabase> m_db;

    public:

        template <typename DefType,
            typename ShapeType = typename ShapeTraits<DefType>::ShapeType>
        tempo_utils::Result<std::shared_ptr<ShapeType>>
        getShape(const SourceDescriptor &source, const std::string &shapeId)
        {
            auto shape = std::make_shared<ShapeType>(getDatabase(), shapeId);
            auto configureStatus = shape->configure(source);
            if (configureStatus.notOk())
                return configureStatus;
            return shape;
        }
    };
}

#endif // GROOVE_SHAPES_SHAPE_PROVIDER_H