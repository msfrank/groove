#ifndef GROOVE_MODEL_INDEXED_PAGE_TEMPLATE_H
#define GROOVE_MODEL_INDEXED_PAGE_TEMPLATE_H

#include <arrow/array.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/message.h>
#include <arrow/ipc/reader.h>
#include <arrow/ipc/writer.h>
#include <arrow/table.h>

#include <groove_data/table_utils.h>

#include "base_page.h"

namespace groove_model {

    template <typename DefType,
        typename KeyType = typename DefType::KeyType,
        typename ValueType = typename DefType::ValueType,
        typename DatumType = typename PageTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::DatumType,
        typename IteratorType = typename PageTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::IteratorType,
        typename VectorType = typename PageTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::VectorType>
    class IndexedPage : public BasePage, public std::enable_shared_from_this<IndexedPage<DefType>> {

    private:
        std::shared_ptr<VectorType> m_vector;

        IndexedPage(
            PageId pageId,
            std::shared_ptr<const std::string> columnId,
            std::shared_ptr<VectorType> vector)
            : BasePage(pageId, columnId),
              m_vector(vector)
        {
        };

    public:

        /**
         *
         * @param key
         * @return
         */
        bool
        hasKey(KeyType key) const
        {
            auto index = groove_data::search_indexed_vector(m_vector, key);
            return 0 <= index;
        }

        /**
         *
         * @param key
         * @param value
         * @return
         */
        bool
        getValue(KeyType key, ValueType &value) const
        {
            auto index = groove_data::search_indexed_vector(m_vector, key);
            if (index < 0)
                return false;
            auto datum = m_vector->getDatum(index);
            if (datum.fidelity != groove_data::DatumFidelity::FIDELITY_VALID)
                return false;
            value = datum.value;
            return true;
        }

        /**
         *
         * @param key
         * @param datum
         * @return
         */
        bool
        getDatum(KeyType key, DatumType &datum) const
        {
            auto index = groove_data::search_indexed_vector(m_vector, key);
            if (index < 0)
                return false;
            datum = m_vector->getDatum(index);
            return true;
        }

        /**
         *
         * @return
         */
        IteratorType
        iterator() const
        {
            return m_vector->iterator();
        }

        /**
         *
         * @return
         */
        int
        numRows() const
        {
            return m_vector->getSize();
        }

        /**
         *
         * @return
         */
        std::shared_ptr<VectorType>
        getVector() const
        {
            return m_vector;
        }

        /**
         *
         * @return
         */
        std::shared_ptr<arrow::Buffer>
        toBuffer() const
        {
            auto createBufResult = arrow::io::BufferOutputStream::Create();
            if (!createBufResult.ok())
                return {};
            auto stream = *createBufResult;

            auto vectorSchema = m_vector->getSchema();
            auto keyField = vectorSchema->field(m_vector->getKeyFieldIndex());
            auto valField = vectorSchema->field(m_vector->getValFieldIndex());
            auto fidField = vectorSchema->field(m_vector->getFidFieldIndex());
            auto schema = arrow::schema({keyField, valField, fidField});

            auto makeWriterResult = arrow::ipc::MakeStreamWriter(stream, schema);
            if (!makeWriterResult.ok())
                return {};
            auto writer = *makeWriterResult;

            auto vectorTable = m_vector->getTable();
            auto keyArray = vectorTable->column(m_vector->getKeyFieldIndex());
            auto valArray = vectorTable->column(m_vector->getValFieldIndex());
            auto fidArray = vectorTable->column(m_vector->getFidFieldIndex());
            auto table = arrow::Table::Make(schema, {keyArray, valArray, fidArray});

            arrow::Status status;
            status = writer->WriteTable(*table);
            if (!status.ok())
                return {};
            status = writer->Close();
            if (!status.ok())
                return {};
            auto finishStreamResult = stream->Finish();
            if (!finishStreamResult.ok())
                return {};
            auto buffer = *finishStreamResult;
            return buffer;
        }

        /**
         *
         * @param pageId
         * @param vector
         * @return
         */
        static std::shared_ptr<IndexedPage<DefType>>
        fromVector(PageId pageId, std::shared_ptr<VectorType> vector)
        {
            if (!pageId.isValid())
                return nullptr;
            if (vector == nullptr)
                return nullptr;
            auto columnId = std::make_shared<const std::string>(vector->getColumnId());
            return std::shared_ptr<IndexedPage<DefType>>(new IndexedPage<DefType>(pageId, columnId, vector));
        }

        /**
         *
         * @param pageId
         * @param bytes
         * @return
         */
        static std::shared_ptr<IndexedPage<DefType>>
        fromBuffer(PageId pageId, std::shared_ptr<arrow::Buffer> buffer)
        {
            auto makeTableResult = groove_data::make_table(buffer);
            if (makeTableResult.isStatus())
                return nullptr;
            auto vector = VectorType::create(makeTableResult.getResult(), 0, 1, 2);
            return fromVector(pageId, vector);
        }

        /**
         *
         * @param pageId
         * @param bytes
         * @return
         */
        static std::shared_ptr<IndexedPage<DefType>>
        fromBytes(PageId pageId, std::shared_ptr<const std::string> bytes)
        {
            auto makeTableResult = groove_data::make_table(bytes);
            if (makeTableResult.isStatus())
                return nullptr;
            auto vector = VectorType::create(makeTableResult.getResult(), 0, 1, 2);
            return fromVector(pageId, vector);
        }
    };
}

#endif // GROOVE_MODEL_INDEXED_PAGE_TEMPLATE_H