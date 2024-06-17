#ifndef GROOVE_DATA_INT64_STRING_VECTOR_H
#define GROOVE_DATA_INT64_STRING_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct Int64StringDatum {
        tu_int64 key;
        std::string value;
        groove_data::DatumFidelity fidelity;
    };

    class Int64StringVector;

    class Int64StringDatumIterator : public Iterator<Int64StringDatum> {
    public:
        Int64StringDatumIterator();
        Int64StringDatumIterator(std::shared_ptr<const Int64StringVector> vector);
        bool getNext(Int64StringDatum &datum) override;

    private:
        std::shared_ptr<const Int64StringVector> m_vector;
        tu_int64 m_curr;
    };

    class Int64StringVector : public BaseVector, public std::enable_shared_from_this<Int64StringVector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        Int64StringDatum getDatum(int index) const;
        Option<Int64StringDatum> getSmallest() const;
        Option<Int64StringDatum> getLargest() const;
        Int64StringDatumIterator iterator() const;
        std::shared_ptr<Int64StringVector> slice(const Int64Range &range) const;

        static std::shared_ptr<Int64StringVector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn);

    private:
        Int64StringVector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_INT64_STRING_VECTOR_H