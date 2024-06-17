
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/category_double_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::CategoryDoubleDatumIterator::CategoryDoubleDatumIterator()
    : m_curr(-1)
{
}

groove_data::CategoryDoubleDatumIterator::CategoryDoubleDatumIterator(std::shared_ptr<const CategoryDoubleVector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::CategoryDoubleDatumIterator::getNext(CategoryDoubleDatum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::CategoryDoubleVector::CategoryDoubleVector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::CategoryDoubleVector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_CATEGORY_DOUBLE;
}

groove_data::DataKeyType
groove_data::CategoryDoubleVector::getKeyType() const
{
    return DataKeyType::KEY_CATEGORY;
}

groove_data::DataValueType
groove_data::CategoryDoubleVector::getValueType() const
{
    return DataValueType::VALUE_TYPE_DOUBLE;
}

groove_data::CategoryDoubleDatum
groove_data::CategoryDoubleVector::getDatum(int index) const
{
    CategoryDoubleDatum datum = {{}, 0, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_category_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_double_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::CategoryDoubleDatum>
groove_data::CategoryDoubleVector::getSmallest() const
{
    if (!isEmpty())
        return Option<CategoryDoubleDatum>(getDatum(0));
    return Option<CategoryDoubleDatum>();
}

Option<groove_data::CategoryDoubleDatum>
groove_data::CategoryDoubleVector::getLargest() const
{
    if (!isEmpty())
        return Option<CategoryDoubleDatum>(getDatum(getSize() - 1));
    return Option<CategoryDoubleDatum>();
}

groove_data::CategoryDoubleDatumIterator
groove_data::CategoryDoubleVector::iterator() const
{
    return CategoryDoubleDatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::CategoryDoubleVector>
groove_data::CategoryDoubleVector::slice(const groove_data::CategoryRange &range) const
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
    return CategoryDoubleVector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::CategoryDoubleVector>
groove_data::CategoryDoubleVector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<CategoryDoubleVector>(new CategoryDoubleVector(table, keyColumn, valueColumn, fidelityColumn));
}