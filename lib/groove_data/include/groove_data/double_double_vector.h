#ifndef GROOVE_DATA_DOUBLE_DOUBLE_VECTOR_H
#define GROOVE_DATA_DOUBLE_DOUBLE_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"

namespace groove_data {

    struct DoubleDoubleDatum {
        double key;
        double value;
        groove_data::DatumFidelity fidelity;
    };

    class DoubleDoubleVector;

    class DoubleDoubleDatumIterator : public Iterator<DoubleDoubleDatum> {
    public:
        DoubleDoubleDatumIterator();
        DoubleDoubleDatumIterator(std::shared_ptr<const DoubleDoubleVector> vector);
        bool getNext(DoubleDoubleDatum &datum) override;

    private:
        std::shared_ptr<const DoubleDoubleVector> m_vector;
        tu_int64 m_curr;
    };

    class DoubleDoubleVector : public BaseVector, public std::enable_shared_from_this<DoubleDoubleVector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        DoubleDoubleDatum getDatum(int index) const;
        Option<DoubleDoubleDatum> getSmallest() const;
        Option<DoubleDoubleDatum> getLargest() const;
        DoubleDoubleDatumIterator iterator() const;
        std::shared_ptr<DoubleDoubleVector> slice(const DoubleRange &range) const;

        static std::shared_ptr<DoubleDoubleVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        DoubleDoubleVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_DOUBLE_DOUBLE_VECTOR_H