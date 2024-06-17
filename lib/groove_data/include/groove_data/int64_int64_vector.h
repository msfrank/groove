#ifndef GROOVE_DATA_INT64_INT64_VECTOR_H
#define GROOVE_DATA_INT64_INT64_VECTOR_H

#include <tempo_utils/iterator_template.h>
#include <tempo_utils/option_template.h>

#include "base_frame.h"
#include "base_vector.h"
#include "data_types.h"

namespace groove_data {

    struct Int64Int64Datum {
        tu_int64 key;
        tu_int64 value;
        groove_data::DatumFidelity fidelity;
    };

    class Int64Int64Vector;

    class Int64Int64DatumIterator : public Iterator<Int64Int64Datum> {
    public:
        Int64Int64DatumIterator();
        Int64Int64DatumIterator(std::shared_ptr<const Int64Int64Vector> vector);
        bool getNext(Int64Int64Datum &datum) override;

    private:
        std::shared_ptr<const Int64Int64Vector> m_vector;
        tu_int64 m_curr;
    };

    class Int64Int64Vector : public BaseVector, public std::enable_shared_from_this<Int64Int64Vector> {
    public:
        DataVectorType getVectorType() const override;
        DataKeyType getKeyType() const override;
        DataValueType getValueType() const override;

        Int64Int64Datum getDatum(int index) const;
        Option<Int64Int64Datum> getSmallest() const;
        Option<Int64Int64Datum> getLargest() const;
        Int64Int64DatumIterator iterator() const;
        std::shared_ptr<Int64Int64Vector> slice(const Int64Range &range) const;

        static std::shared_ptr<Int64Int64Vector> create(
            std::shared_ptr<arrow::Table> table,
            int keyColumn,
            int valColumn,
            int fidColumn);

    private:
        Int64Int64Vector(std::shared_ptr<arrow::Table> table, int keyColumn, int valColumn, int fidColumn);
    };
}

#endif // GROOVE_DATA_INT64_INT64_VECTOR_H