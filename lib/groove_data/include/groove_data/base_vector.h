#ifndef GROOVE_DATA_BASE_VECTOR_H
#define GROOVE_DATA_BASE_VECTOR_H

#include <arrow/table.h>

#include "data_types.h"

namespace groove_data {

    class BaseVector {

    public:
        virtual ~BaseVector() = default;

        std::shared_ptr<arrow::Table> getTable() const;
        std::shared_ptr<arrow::Schema> getSchema() const;
        std::string getColumnId() const;
        int getKeyFieldIndex() const;
        int getValFieldIndex() const;
        int getFidFieldIndex() const;
        bool isEmpty() const;
        int getSize() const;

        virtual DataVectorType getVectorType() const = 0;
        virtual DataKeyType getKeyType() const = 0;
        virtual DataValueType getValueType() const = 0;

        std::string toString() const;
        void print() const;

    protected:
        explicit BaseVector(
            std::shared_ptr<arrow::Table> table,
            int keyFieldIndex,
            int valFieldIndex,
            int fidFieldIndex);

    private:
        std::shared_ptr<arrow::Table> m_table;
        int m_keyFieldIndex;
        int m_valFieldIndex;
        int m_fidFieldIndex;
    };
}

#endif // GROOVE_DATA_BASE_VECTOR_H