
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/category_int64_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::CategoryInt64DatumIterator::CategoryInt64DatumIterator()
    : m_curr(-1)
{
}

groove_data::CategoryInt64DatumIterator::CategoryInt64DatumIterator(std::shared_ptr<const CategoryInt64Vector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::CategoryInt64DatumIterator::getNext(CategoryInt64Datum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::CategoryInt64Vector::CategoryInt64Vector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::CategoryInt64Vector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_CATEGORY_INT64;
}

groove_data::DataKeyType
groove_data::CategoryInt64Vector::getKeyType() const
{
    return DataKeyType::KEY_CATEGORY;
}

groove_data::DataValueType
groove_data::CategoryInt64Vector::getValueType() const
{
    return DataValueType::VALUE_TYPE_INT64;
}

groove_data::CategoryInt64Datum
groove_data::CategoryInt64Vector::getDatum(int index) const
{
    CategoryInt64Datum datum = {{}, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_category_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_int64_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::CategoryInt64Datum>
groove_data::CategoryInt64Vector::getSmallest() const
{
    if (!isEmpty())
        return Option<CategoryInt64Datum>(getDatum(0));
    return Option<CategoryInt64Datum>();
}

Option<groove_data::CategoryInt64Datum>
groove_data::CategoryInt64Vector::getLargest() const
{
    if (!isEmpty())
        return Option<CategoryInt64Datum>(getDatum(getSize() - 1));
    return Option<CategoryInt64Datum>();
}

groove_data::CategoryInt64DatumIterator
groove_data::CategoryInt64Vector::iterator() const
{
    return CategoryInt64DatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::CategoryInt64Vector>
groove_data::CategoryInt64Vector::slice(const groove_data::CategoryRange &range) const
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
    return CategoryInt64Vector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::CategoryInt64Vector>
groove_data::CategoryInt64Vector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<CategoryInt64Vector>(new CategoryInt64Vector(table, keyColumn, valueColumn, fidelityColumn));
}