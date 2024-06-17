
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/int64_string_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::Int64StringDatumIterator::Int64StringDatumIterator()
    : m_curr(-1)
{
}

groove_data::Int64StringDatumIterator::Int64StringDatumIterator(std::shared_ptr<const Int64StringVector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::Int64StringDatumIterator::getNext(Int64StringDatum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::Int64StringVector::Int64StringVector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::Int64StringVector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_INT64_STRING;
}

groove_data::DataKeyType
groove_data::Int64StringVector::getKeyType() const
{
    return DataKeyType::KEY_INT64;
}

groove_data::DataValueType
groove_data::Int64StringVector::getValueType() const
{
    return DataValueType::VALUE_TYPE_STRING;
}

groove_data::Int64StringDatum
groove_data::Int64StringVector::getDatum(int index) const
{
    Int64StringDatum datum = {0, {}, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_int64_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_string_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::Int64StringDatum>
groove_data::Int64StringVector::getSmallest() const
{
    if (!isEmpty())
        return Option<Int64StringDatum>(getDatum(0));
    return Option<Int64StringDatum>();
}

Option<groove_data::Int64StringDatum>
groove_data::Int64StringVector::getLargest() const
{
    if (!isEmpty())
        return Option<Int64StringDatum>(getDatum(getSize() - 1));
    return Option<Int64StringDatum>();
}

groove_data::Int64StringDatumIterator
groove_data::Int64StringVector::iterator() const
{
    return Int64StringDatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::Int64StringVector>
groove_data::Int64StringVector::slice(const groove_data::Int64Range &range) const
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
    return Int64StringVector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::Int64StringVector>
groove_data::Int64StringVector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<Int64StringVector>(new Int64StringVector(table, keyColumn, valueColumn, fidelityColumn));
}