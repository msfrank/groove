#include <tempo_utils/logging.h>

#include "info_table_shape.h"

InfoTableShape::InfoTableShape(
    const QString &shapeId,
    const StreamSourceCollection &sources,
    std::shared_ptr<tempo_utils::ConfigMap> properties,
    QObject *parent)
    : BaseDataShape(shapeId, sources, properties, parent)
{
}

bool
InfoTableShape::isValid() const
{
    return true;
}

streams::DataShapeType
InfoTableShape::getShapeType() const
{
    return streams::DataShapeType::SHAPE_INFO_TABLE;
}

ShapeDescriptor
InfoTableShape::getShapeDescriptor() const
{
    return ShapeDescriptor();
}

