
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/int64_int64_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::Int64Int64DatumIterator::Int64Int64DatumIterator()
    : m_curr(-1)
{
}

groove_data::Int64Int64DatumIterator::Int64Int64DatumIterator(std::shared_ptr<const Int64Int64Vector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::Int64Int64DatumIterator::getNext(Int64Int64Datum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::Int64Int64Vector::Int64Int64Vector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::Int64Int64Vector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_INT64_INT64;
}

groove_data::DataKeyType
groove_data::Int64Int64Vector::getKeyType() const
{
    return DataKeyType::KEY_INT64;
}

groove_data::DataValueType
groove_data::Int64Int64Vector::getValueType() const
{
    return DataValueType::VALUE_TYPE_INT64;
}

groove_data::Int64Int64Datum
groove_data::Int64Int64Vector::getDatum(int index) const
{
    Int64Int64Datum datum = {0, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_int64_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_int64_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::Int64Int64Datum>
groove_data::Int64Int64Vector::getSmallest() const
{
    if (!isEmpty())
        return Option<Int64Int64Datum>(getDatum(0));
    return Option<Int64Int64Datum>();
}

Option<groove_data::Int64Int64Datum>
groove_data::Int64Int64Vector::getLargest() const
{
    if (!isEmpty())
        return Option<Int64Int64Datum>(getDatum(getSize() - 1));
    return Option<Int64Int64Datum>();
}

groove_data::Int64Int64DatumIterator
groove_data::Int64Int64Vector::iterator() const
{
    return Int64Int64DatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::Int64Int64Vector>
groove_data::Int64Int64Vector::slice(const groove_data::Int64Range &range) const
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
    return Int64Int64Vector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::Int64Int64Vector>
groove_data::Int64Int64Vector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<Int64Int64Vector>(new Int64Int64Vector(table, keyColumn, valueColumn, fidelityColumn));
}