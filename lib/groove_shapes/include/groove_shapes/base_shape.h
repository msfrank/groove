#ifndef GROOVE_SHAPES_BASE_SHAPE_H
#define GROOVE_SHAPES_BASE_SHAPE_H

#include <groove_model/groove_database.h>

#include "shape_types.h"
#include "shapes_result.h"

namespace groove_shapes {

    class BaseShape {

    public:
        BaseShape(
            std::shared_ptr<groove_model::GrooveDatabase> database,
            const std::string &shapeId);
        virtual ~BaseShape() = default;

        virtual tempo_utils::Status configure(const SourceDescriptor &source) = 0;
        virtual DataShapeType getShapeType() const = 0;

        std::shared_ptr<groove_model::GrooveDatabase> getDatabase() const;
        std::shared_ptr<groove_model::AbstractDataset> getDataset(const tempo_utils::Url &datasetUrl) const;
        std::shared_ptr<groove_model::GrooveModel> getModel(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId) const;

    private:
        std::shared_ptr<groove_model::GrooveDatabase> m_database;
        std::string m_shapeId;
    };
}

#endif // GROOVE_SHAPES_BASE_SHAPE_H