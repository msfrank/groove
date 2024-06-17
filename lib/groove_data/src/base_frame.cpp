
#include <groove_data/base_frame.h>

groove_data::BaseFrame::BaseFrame(std::shared_ptr<arrow::Table> table)
    : m_table(table)
{
    TU_ASSERT (m_table != nullptr);
}

bool
groove_data::BaseFrame::isEmpty() const
{
    return m_table->num_rows() == 0;
}

int
groove_data::BaseFrame::getSize() const
{
    return m_table->num_rows();
}

int
groove_data::BaseFrame::numVectors() const
{
    return m_vectors.size();
}

bool
groove_data::BaseFrame::hasVector(const std::string &columnId) const
{
    return m_vectors.contains(columnId);
}

std::shared_ptr<groove_data::BaseVector>
groove_data::BaseFrame::getVector(const std::string &columnId) const
{
    if (m_vectors.contains(columnId))
        return m_vectors.at(columnId);
    return {};
}

absl::flat_hash_map<std::string,std::shared_ptr<groove_data::BaseVector>>::const_iterator
groove_data::BaseFrame::vectorsBegin() const
{
    return m_vectors.cbegin();
}

absl::flat_hash_map<std::string,std::shared_ptr<groove_data::BaseVector>>::const_iterator
groove_data::BaseFrame::vectorsEnd() const
{
    return m_vectors.cend();
}

void
groove_data::BaseFrame::insertVector(
    const std::string &columnId,
    std::shared_ptr<groove_data::BaseVector> vector)
{
    m_vectors[columnId] = vector;
}

std::shared_ptr<arrow::Table>
groove_data::BaseFrame::getTable() const
{
    return m_table;
}

std::shared_ptr<const arrow::Table>
groove_data::BaseFrame::getUnderlyingTable() const
{
    return m_table;
}
