
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/double_int64_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::DoubleInt64DatumIterator::DoubleInt64DatumIterator()
    : m_curr(-1)
{
}

groove_data::DoubleInt64DatumIterator::DoubleInt64DatumIterator(std::shared_ptr<const DoubleInt64Vector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::DoubleInt64DatumIterator::getNext(DoubleInt64Datum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::DoubleInt64Vector::DoubleInt64Vector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::DoubleInt64Vector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_DOUBLE_INT64;
}

groove_data::DataKeyType
groove_data::DoubleInt64Vector::getKeyType() const
{
    return DataKeyType::KEY_DOUBLE;
}

groove_data::DataValueType
groove_data::DoubleInt64Vector::getValueType() const
{
    return DataValueType::VALUE_TYPE_INT64;
}

groove_data::DoubleInt64Datum
groove_data::DoubleInt64Vector::getDatum(int index) const
{
    DoubleInt64Datum datum = {0, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_double_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_int64_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::DoubleInt64Datum>
groove_data::DoubleInt64Vector::getSmallest() const
{
    if (!isEmpty())
        return Option<DoubleInt64Datum>(getDatum(0));
    return Option<DoubleInt64Datum>();
}

Option<groove_data::DoubleInt64Datum>
groove_data::DoubleInt64Vector::getLargest() const
{
    if (!isEmpty())
        return Option<DoubleInt64Datum>(getDatum(getSize() - 1));
    return Option<DoubleInt64Datum>();
}

groove_data::DoubleInt64DatumIterator
groove_data::DoubleInt64Vector::iterator() const
{
    return DoubleInt64DatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::DoubleInt64Vector>
groove_data::DoubleInt64Vector::slice(const groove_data::DoubleRange &range) const
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
    return DoubleInt64Vector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::DoubleInt64Vector>
groove_data::DoubleInt64Vector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<DoubleInt64Vector>(new DoubleInt64Vector(table, keyColumn, valueColumn, fidelityColumn));
}