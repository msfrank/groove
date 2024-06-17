#ifndef GROOVE_DATA_CATEGORY_INT64_VECTOR_H
#define GROOVE_DATA_CATEGORY_INT64_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct CategoryInt64Datum {
        Category key;
        tu_int64 value;
        groove_data::DatumFidelity fidelity;
    };

    class CategoryInt64Vector;

    class CategoryInt64DatumIterator : public Iterator<CategoryInt64Datum> {
    public:
        CategoryInt64DatumIterator();
        CategoryInt64DatumIterator(std::shared_ptr<const CategoryInt64Vector> vector);
        bool getNext(CategoryInt64Datum &datum) override;

    private:
        std::shared_ptr<const CategoryInt64Vector> m_vector;
        tu_int64 m_curr;
    };

    class CategoryInt64Vector : public BaseVector, public std::enable_shared_from_this<CategoryInt64Vector> {
    public:
        groove_data::DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        CategoryInt64Datum getDatum(int index) const;
        Option<CategoryInt64Datum> getSmallest() const;
        Option<CategoryInt64Datum> getLargest() const;
        CategoryInt64DatumIterator iterator() const;
        std::shared_ptr<CategoryInt64Vector> slice(const CategoryRange &range) const;

        static std::shared_ptr<CategoryInt64Vector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        CategoryInt64Vector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_CATEGORY_INT64_VECTOR_H