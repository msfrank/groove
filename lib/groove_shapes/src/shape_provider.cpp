
#include <groove_shapes/shape_provider.h>

groove_shapes::ShapeProvider::ShapeProvider(std::shared_ptr<groove_model::GrooveDatabase> db)
    : m_db(db)
{
    TU_ASSERT (m_db != nullptr);
}

std::shared_ptr<groove_model::GrooveDatabase>
groove_shapes::ShapeProvider::getDatabase() const
{
    return m_db;
}
