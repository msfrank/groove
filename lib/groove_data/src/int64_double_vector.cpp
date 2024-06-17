
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/int64_double_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::Int64DoubleDatumIterator::Int64DoubleDatumIterator()
    : m_curr(-1)
{
}

groove_data::Int64DoubleDatumIterator::Int64DoubleDatumIterator(std::shared_ptr<const Int64DoubleVector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::Int64DoubleDatumIterator::getNext(Int64DoubleDatum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::Int64DoubleVector::Int64DoubleVector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::Int64DoubleVector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_INT64_DOUBLE;
}

groove_data::DataKeyType
groove_data::Int64DoubleVector::getKeyType() const
{
    return DataKeyType::KEY_INT64;
}

groove_data::DataValueType
groove_data::Int64DoubleVector::getValueType() const
{
    return DataValueType::VALUE_TYPE_DOUBLE;
}

groove_data::Int64DoubleDatum
groove_data::Int64DoubleVector::getDatum(int index) const
{
    Int64DoubleDatum datum = {0, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_int64_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_double_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::Int64DoubleDatum>
groove_data::Int64DoubleVector::getSmallest() const
{
    if (!isEmpty())
        return Option<Int64DoubleDatum>(getDatum(0));
    return Option<Int64DoubleDatum>();
}

Option<groove_data::Int64DoubleDatum>
groove_data::Int64DoubleVector::getLargest() const
{
    if (!isEmpty())
        return Option<Int64DoubleDatum>(getDatum(getSize() - 1));
    return Option<Int64DoubleDatum>();
}

groove_data::Int64DoubleDatumIterator
groove_data::Int64DoubleVector::iterator() const
{
    return Int64DoubleDatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::Int64DoubleVector>
groove_data::Int64DoubleVector::slice(const groove_data::Int64Range &range) const
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
    return Int64DoubleVector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::Int64DoubleVector>
groove_data::Int64DoubleVector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<Int64DoubleVector>(new Int64DoubleVector(table, keyColumn, valueColumn, fidelityColumn));
}