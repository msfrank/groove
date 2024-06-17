#ifndef GROOVE_DATA_DOUBLE_STRING_VECTOR_H
#define GROOVE_DATA_DOUBLE_STRING_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"

namespace groove_data {

    struct DoubleStringDatum {
        double key;
        std::string value;
        groove_data::DatumFidelity fidelity;
    };

    class DoubleStringVector;

    class DoubleStringDatumIterator : public Iterator<DoubleStringDatum> {
    public:
        DoubleStringDatumIterator();
        DoubleStringDatumIterator(std::shared_ptr<const DoubleStringVector> vector);
        bool getNext(DoubleStringDatum &datum) override;

    private:
        std::shared_ptr<const DoubleStringVector> m_vector;
        tu_int64 m_curr;
    };

    class DoubleStringVector : public BaseVector, public std::enable_shared_from_this<DoubleStringVector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        DoubleStringDatum getDatum(int index) const;
        Option<DoubleStringDatum> getSmallest() const;
        Option<DoubleStringDatum> getLargest() const;
        DoubleStringDatumIterator iterator() const;
        std::shared_ptr<DoubleStringVector> slice(const DoubleRange &range) const;

        static std::shared_ptr<DoubleStringVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn = -1);

    private:
        DoubleStringVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_DOUBLE_STRING_VECTOR_H