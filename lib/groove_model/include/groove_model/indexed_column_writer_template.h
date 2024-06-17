#ifndef GROOVE_MODEL_INDEXED_COLUMN_WRITER_TEMPLATE_H
#define GROOVE_MODEL_INDEXED_COLUMN_WRITER_TEMPLATE_H

#include <arrow/builder.h>

#include <groove_data/base_vector.h>

#include "abstract_page_store.h"
#include "base_column.h"
#include "model_result.h"
#include "model_types.h"

namespace groove_model {

    template <typename DefType,
        typename KeyType = typename DefType::KeyType,
        typename ValueType = typename DefType::ValueType,
        typename KeyBuilderType = typename DefType::KeyBuilderType,
        typename ValueBuilderType = typename DefType::ValueBuilderType,
        typename DatumType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::DatumType,
        typename IteratorType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::IteratorType,
        typename RangeType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::RangeType,
        typename VectorType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::VectorType>
    class IndexedColumnWriter : public BaseColumn, public std::enable_shared_from_this<IndexedColumnWriter<DefType>> {

    private:
        std::shared_ptr<AbstractPageStore> m_pageStore;

        IndexedColumnWriter(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            std::shared_ptr<AbstractPageStore> pageStore)
            : BaseColumn(datasetUrl, modelId, columnId),
              m_pageStore(pageStore)
        {
        };

    public:

        /**
         *
         * @param range
         * @return
         */
        tempo_utils::Result<IteratorType>
        getValues(const RangeType &range)
        {
            std::forward_list<std::shared_ptr<VectorType>> vectors;
            std::vector<PageId> pageIds;
            Option<KeyType> key = range.start;

            auto getIndexedPageResult = m_pageStore->template getIndexedPage<DefType>(
                getDatasetUrl(), getModelId(), getColumnId(), key, false);
            if (getIndexedPageResult.isStatus()) {
                auto status = getIndexedPageResult.getStatus();
                if (!status.matchesCondition(ModelCondition::kPageNotFound))
                    return status;
                return IteratorType();
            }
            auto page = getIndexedPageResult.getResult();

            auto vector = page->getVector();
            auto slice = vector->slice(range);
            vectors.push_front(slice);
            pageIds.push_back(page->getPageId());
            auto last = vectors.begin();

            auto largestOption = slice->getLargest();
            while (!largestOption.isEmpty()) {
                auto largest = largestOption.getValue();
                getIndexedPageResult = m_pageStore->template getIndexedPage<DefType>(
                    getDatasetUrl(), getModelId(), getColumnId(), Option<KeyType>(largest.key), true);
                if (getIndexedPageResult.isStatus()) {
                    auto status = getIndexedPageResult.getStatus();
                    if (status.matchesCondition(ModelCondition::kPageNotFound))
                        break;
                    return status;
                }
                page = getIndexedPageResult.getResult();

                vector = page->getVector();
                slice = vector->slice(range);
                if (slice->isEmpty())
                    break;
                last = vectors.insert_after(last, slice);
                pageIds.push_back(page->getPageId());
                largestOption = slice->getLargest();
            }

            return IteratorType(vectors, pageIds);
        };

        /**
         *
         * @param vector
         * @return
         */
        tempo_utils::Status
        setValues(std::shared_ptr<VectorType> vector)
        {
            TU_ASSERT (vector != nullptr);
            if (vector->isEmpty())
                return ModelStatus::ok();

            DatumType smallest = vector->getSmallest().getValue();
            DatumType largest = vector->getLargest().getValue();
            RangeType range;
            range.start = Option<KeyType>(smallest.key);
            range.start_exclusive = false;
            range.end = Option<KeyType>(largest.key);
            range.end_exclusive = false;

            // get iterator containing all pages holding data for range
            auto getCurrentResult = getValues(range);
            if (getCurrentResult.isStatus())
                return getCurrentResult.getStatus();
            auto current = getCurrentResult.getResult();

            // get iterator containing updates
            auto updates = vector->iterator();

            // set initial input state
            DatumType currentDatum;
            bool hasCurrent = current.getNext(currentDatum);
            DatumType updatedDatum;
            bool hasupdated = updates.getNext(updatedDatum);

            // set initial output state
            auto schema = vector->getSchema();
            KeyBuilderType keyBuilder;
            ValueBuilderType valBuilder;
            arrow::BooleanBuilder fidBuilder;

            //
            tu_int64 numRows = 0;
            while (hasCurrent || hasupdated) {
                arrow::Status status;
                if (hasCurrent && (!hasupdated || currentDatum.key < updatedDatum.key)) {
                    status = keyBuilder.Append(currentDatum.key);
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    status = valBuilder.Append(currentDatum.value);
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    status = fidBuilder.AppendNull();
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    numRows++;
                    hasCurrent = current.getNext(currentDatum);
                } else {
                    if (hasCurrent && currentDatum.key == updatedDatum.key) {
                        hasCurrent = current.getNext(currentDatum);
                    }
                    status = keyBuilder.Append(updatedDatum.key);
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    status = valBuilder.Append(updatedDatum.value);
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    status = fidBuilder.AppendNull();
                    if (!status.ok())
                        return ModelStatus::forCondition(ModelCondition::kModelInvariant, status.ToString());
                    numRows++;
                    hasupdated = updates.getNext(updatedDatum);
                }
            }

            // construct the merged vector
            auto finishKeyResult = keyBuilder.Finish();
            if (!finishKeyResult.ok())
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, finishKeyResult.status().ToString());
            auto finishValResult = valBuilder.Finish();
            if (!finishValResult.ok())
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, finishValResult.status().ToString());
            auto finishFidResult = fidBuilder.Finish();
            if (!finishFidResult.ok())
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, finishFidResult.status().ToString());
            auto table = arrow::Table::Make(schema, {*finishKeyResult, *finishValResult, *finishFidResult}, numRows);
            auto merged = VectorType::create(table, 0, 1, 2);

            // construct the page
            auto pageId = PageId::create<DefType,groove_data::CollationMode::COLLATION_INDEXED>(
                getDatasetUrl(), getModelId(), getColumnId(), Option<KeyType>(merged->getSmallest().getValue().key));

            auto page = IndexedPage<DefType>::fromVector(pageId, merged);

            std::unique_ptr<AbstractPageStoreTransaction> txn(m_pageStore->startTransaction());
            if (txn == nullptr)
                return ModelStatus::forCondition(ModelCondition::kModelInvariant, "failed to start transaction");

            // remove old pages from the persistent store
            for (auto iterator = current.pageIdsBegin(); iterator != current.pageIdsEnd(); iterator++) {
                auto currPageId = *iterator;
                auto status = txn->removePage(currPageId);
                if (status.notOk())
                    return status;
            }

            // insert new page into the persistent store
            auto buffer = page->toBuffer();
            auto status = txn->writePage(pageId, buffer);
            if (status.notOk())
                return status;

            // apply store changes atomically
            status = txn->apply();
            if (status.notOk())
                return status;

            return ModelStatus::ok();
        };

        /**
         *
         * @param modelId
         * @param columnId
         * @param store
         * @return
         */
        static std::shared_ptr<IndexedColumnWriter<DefType>>
        create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            std::shared_ptr<AbstractPageStore> pageStore)
        {
            return std::shared_ptr<IndexedColumnWriter<DefType>>(
                new IndexedColumnWriter<DefType>(datasetUrl, modelId, columnId, pageStore));
        };
    };
}

#endif // GROOVE_MODEL_INDEXED_COLUMN_WRITER_TEMPLATE_H