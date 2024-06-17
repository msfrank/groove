#ifndef GROOVE_DATA_CATEGORY_DOUBLE_VECTOR_H
#define GROOVE_DATA_CATEGORY_DOUBLE_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct CategoryDoubleDatum {
        Category key;
        double value;
        groove_data::DatumFidelity fidelity;
    };

    class CategoryDoubleVector;

    class CategoryDoubleDatumIterator : public Iterator<CategoryDoubleDatum> {
    public:
        CategoryDoubleDatumIterator();
        CategoryDoubleDatumIterator(std::shared_ptr<const CategoryDoubleVector> vector);
        bool getNext(CategoryDoubleDatum &datum) override;

    private:
        std::shared_ptr<const CategoryDoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class CategoryDoubleVector : public BaseVector, public std::enable_shared_from_this<CategoryDoubleVector> {
    public:
        groove_data::DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        CategoryDoubleDatum getDatum(int index) const;
        Option<CategoryDoubleDatum> getSmallest() const;
        Option<CategoryDoubleDatum> getLargest() const;
        CategoryDoubleDatumIterator iterator() const;
        std::shared_ptr<CategoryDoubleVector> slice(const CategoryRange &range) const;

        static std::shared_ptr<CategoryDoubleVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        CategoryDoubleVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_CATEGORY_DOUBLE_VECTOR_H