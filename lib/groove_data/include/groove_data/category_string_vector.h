#ifndef GROOVE_DATA_CATEGORY_STRING_VECTOR_H
#define GROOVE_DATA_CATEGORY_STRING_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct CategoryStringDatum {
        Category key;
        std::string value;
        groove_data::DatumFidelity fidelity;
    };

    class CategoryStringVector;

    class CategoryStringDatumIterator : public Iterator<CategoryStringDatum> {
    public:
        CategoryStringDatumIterator();
        CategoryStringDatumIterator(std::shared_ptr<const CategoryStringVector> vector);
        bool getNext(CategoryStringDatum &datum) override;

    private:
        std::shared_ptr<const CategoryStringVector> m_vector;
        tu_int64 m_curr;
    };

    class CategoryStringVector : public BaseVector, public std::enable_shared_from_this<CategoryStringVector> {
    public:
        groove_data::DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        CategoryStringDatum getDatum(int index) const;
        Option<CategoryStringDatum> getSmallest() const;
        Option<CategoryStringDatum> getLargest() const;
        CategoryStringDatumIterator iterator() const;
        std::shared_ptr<CategoryStringVector> slice(const CategoryRange &range) const;

        static std::shared_ptr<CategoryStringVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        CategoryStringVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_CATEGORY_STRING_VECTOR_H