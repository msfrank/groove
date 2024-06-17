#ifndef GROOVE_DATA_BASE_FRAME_H
#define GROOVE_DATA_BASE_FRAME_H

#include <string>

#include <absl/container/flat_hash_map.h>

#include "base_vector.h"

namespace groove_data {

    class BaseFrame {

    public:
        virtual ~BaseFrame() = default;

        bool isEmpty() const;
        int getSize() const;

        virtual DataFrameType getFrameType() const = 0;
        virtual DataKeyType getKeyType() const = 0;
        virtual int getKeyFieldIndex() const = 0;

        bool hasVector(const std::string &columnId) const;
        std::shared_ptr<BaseVector> getVector(const std::string &columnId) const;
        int numVectors() const;

        absl::flat_hash_map<std::string, std::shared_ptr<BaseVector>>::const_iterator vectorsBegin() const;
        absl::flat_hash_map<std::string, std::shared_ptr<BaseVector>>::const_iterator vectorsEnd() const;

        std::shared_ptr<const arrow::Table> getUnderlyingTable() const;

    protected:
        explicit BaseFrame(std::shared_ptr<arrow::Table> table);
        std::shared_ptr<arrow::Table> getTable() const;
        void insertVector(const std::string &columnId, std::shared_ptr<BaseVector> vector);

    private:
        std::shared_ptr<arrow::Table> m_table;
        absl::flat_hash_map<std::string, std::shared_ptr<BaseVector>> m_vectors;
    };
}

#endif // GROOVE_DATA_BASE_FRAME_H