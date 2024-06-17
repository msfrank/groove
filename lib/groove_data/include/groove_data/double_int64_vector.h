#ifndef GROOVE_DATA_DOUBLE_INT64_VECTOR_H
#define GROOVE_DATA_DOUBLE_INT64_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"

namespace groove_data {

    struct DoubleInt64Datum {
        double key;
        tu_int64 value;
        groove_data::DatumFidelity fidelity;
    };

    class DoubleInt64Vector;

    class DoubleInt64DatumIterator : public Iterator<DoubleInt64Datum> {
    public:
        DoubleInt64DatumIterator();
        DoubleInt64DatumIterator(std::shared_ptr<const DoubleInt64Vector> vector);
        bool getNext(DoubleInt64Datum &datum) override;

    private:
        std::shared_ptr<const DoubleInt64Vector> m_vector;
        tu_int64 m_curr;
    };

    class DoubleInt64Vector : public BaseVector, public std::enable_shared_from_this<DoubleInt64Vector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        DoubleInt64Datum getDatum(int index) const;
        Option<DoubleInt64Datum> getSmallest() const;
        Option<DoubleInt64Datum> getLargest() const;
        DoubleInt64DatumIterator iterator() const;
        std::shared_ptr<DoubleInt64Vector> slice(const DoubleRange &range) const;

        static std::shared_ptr<DoubleInt64Vector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        DoubleInt64Vector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_DOUBLE_INT64_VECTOR_H