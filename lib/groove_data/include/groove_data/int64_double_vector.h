#ifndef GROOVE_DATA_INT64_DOUBLE_VECTOR_H
#define GROOVE_DATA_INT64_DOUBLE_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct Int64DoubleDatum {
        tu_int64 key;
        double value;
        groove_data::DatumFidelity fidelity;
    };

    class Int64DoubleVector;

    class Int64DoubleDatumIterator : public Iterator<Int64DoubleDatum> {
    public:
        Int64DoubleDatumIterator();
        Int64DoubleDatumIterator(std::shared_ptr<const Int64DoubleVector> vector);
        bool getNext(Int64DoubleDatum &datum) override;

    private:
        std::shared_ptr<const Int64DoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class Int64DoubleVector : public BaseVector, public std::enable_shared_from_this<Int64DoubleVector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        Int64DoubleDatum getDatum(int index) const;
        Option<Int64DoubleDatum> getSmallest() const;
        Option<Int64DoubleDatum> getLargest() const;
        Int64DoubleDatumIterator iterator() const;
        std::shared_ptr<Int64DoubleVector> slice(const Int64Range &range) const;

        static std::shared_ptr<Int64DoubleVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn);

    private:
        Int64DoubleVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_INT64_DOUBLE_VECTOR_H