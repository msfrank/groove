
#include <arrow/array.h>
#include <arrow/chunked_array.h>

#include <groove_data/array_utils.h>
#include <groove_data/category_string_vector.h>
#include <groove_data/data_types.h>
#include <tempo_utils/logging.h>

groove_data::CategoryStringDatumIterator::CategoryStringDatumIterator()
    : m_curr(-1)
{
}

groove_data::CategoryStringDatumIterator::CategoryStringDatumIterator(std::shared_ptr<const CategoryStringVector> vector)
    : m_vector(vector),
      m_curr(0)
{
    TU_ASSERT (m_vector != nullptr);
}

bool
groove_data::CategoryStringDatumIterator::getNext(CategoryStringDatum &datum)
{
    if (!m_vector || m_vector->getSize() <= m_curr)
        return false;
    datum = m_vector->getDatum(m_curr++);
    return true;
}

groove_data::CategoryStringVector::CategoryStringVector(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valColumn,
    int fidColumn)
    : BaseVector(table, keyColumn, valColumn, fidColumn)
{
}

groove_data::DataVectorType
groove_data::CategoryStringVector::getVectorType() const
{
    return DataVectorType::VECTOR_TYPE_CATEGORY_STRING;
}

groove_data::DataKeyType
groove_data::CategoryStringVector::getKeyType() const
{
    return DataKeyType::KEY_CATEGORY;
}

groove_data::DataValueType
groove_data::CategoryStringVector::getValueType() const
{
    return DataValueType::VALUE_TYPE_STRING;
}

groove_data::CategoryStringDatum
groove_data::CategoryStringVector::getDatum(int index) const
{
    CategoryStringDatum datum = {{}, {}, DatumFidelity::FIDELITY_INVALID};

    auto table = getTable();
    if (table->num_rows() <= index)
        return datum;
    auto keyArray = table->column(getKeyFieldIndex());
    auto valArray = table->column(getValFieldIndex());
    if (!groove_data::get_category_datum(keyArray, index, datum.key))
        return datum;
    if (!groove_data::get_string_datum(valArray, index, datum.value))
        return datum;
    datum.fidelity = DatumFidelity::FIDELITY_VALID;
    return datum;
}

Option<groove_data::CategoryStringDatum>
groove_data::CategoryStringVector::getSmallest() const
{
    if (!isEmpty())
        return Option<CategoryStringDatum>(getDatum(0));
    return Option<CategoryStringDatum>();
}

Option<groove_data::CategoryStringDatum>
groove_data::CategoryStringVector::getLargest() const
{
    if (!isEmpty())
        return Option<CategoryStringDatum>(getDatum(getSize() - 1));
    return Option<CategoryStringDatum>();
}

groove_data::CategoryStringDatumIterator
groove_data::CategoryStringVector::iterator() const
{
    return CategoryStringDatumIterator(shared_from_this());
}

std::shared_ptr<groove_data::CategoryStringVector>
groove_data::CategoryStringVector::slice(const groove_data::CategoryRange &range) const
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
    return CategoryStringVector::create(view, getKeyFieldIndex(), getValFieldIndex(), getFidFieldIndex());
}

std::shared_ptr<groove_data::CategoryStringVector>
groove_data::CategoryStringVector::create(
    std::shared_ptr<arrow::Table> table,
    int keyColumn,
    int valueColumn,
    int fidelityColumn)
{
    return std::shared_ptr<CategoryStringVector>(new CategoryStringVector(table, keyColumn, valueColumn, fidelityColumn));
}