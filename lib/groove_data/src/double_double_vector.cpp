
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/double_double_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::DoubleDoubleDatumIterator::DoubleDoubleDatumIterator()
    : m_curr(-1)
{
}

groove_data::DoubleDoubleDatumIterator::DoubleDoubleDatumIterator(std::shared_ptr<const DoubleDoubleVector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::DoubleDoubleDatumIterator::getNext(DoubleDoubleDatum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::DoubleDoubleVector::DoubleDoubleVector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::DoubleDoubleVector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_DOUBLE_DOUBLE;
}

groove_data::DataKeyType
groove_data::DoubleDoubleVector::getKeyType() const
{
    return DataKeyType::KEY_DOUBLE;
}

groove_data::DataValueType
groove_data::DoubleDoubleVector::getValueType() const
{
    return DataValueType::VALUE_TYPE_DOUBLE;
}

groove_data::DoubleDoubleDatum
groove_data::DoubleDoubleVector::getDatum(int index) const
{
    DoubleDoubleDatum datum = {0, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_double_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_double_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::DoubleDoubleDatum>
groove_data::DoubleDoubleVector::getSmallest() const
{
    if (!isEmpty())
        return Option<DoubleDoubleDatum>(getDatum(0));
    return Option<DoubleDoubleDatum>();
}

Option<groove_data::DoubleDoubleDatum>
groove_data::DoubleDoubleVector::getLargest() const
{
    if (!isEmpty())
        return Option<DoubleDoubleDatum>(getDatum(getSize() - 1));
    return Option<DoubleDoubleDatum>();
}

groove_data::DoubleDoubleDatumIterator
groove_data::DoubleDoubleVector::iterator() const
{
    return DoubleDoubleDatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::DoubleDoubleVector>
groove_data::DoubleDoubleVector::slice(const groove_data::DoubleRange &range) const
{
    auto vector = shared_from_this();
    if (isEmpty())
        return groove_data::empty(vector);

    int startIndex = find_start_index(vector, range);
    if (startIndex < 0)
        return groove_data::empty(vector);
    int endIndex = find_end_index(vector, range);
    if (endIndex < 0)
        return groove_data::empty(vector);
    TU_ASSERT (startIndex <= endIndex);
    auto count = (endIndex - startIndex) + 1;

    auto table = getTable();
    auto view = table->Slice(startIndex, count);
    return DoubleDoubleVector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::DoubleDoubleVector>
groove_data::DoubleDoubleVector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<DoubleDoubleVector>(new DoubleDoubleVector(table, keyColumn, valueColumn, fidelityColumn));
}