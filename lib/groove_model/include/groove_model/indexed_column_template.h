#ifndef GROOVE_MODEL_INDEXED_COLUMN_TEMPLATE_H
#define GROOVE_MODEL_INDEXED_COLUMN_TEMPLATE_H

#include <arrow/builder.h>

#include <groove_data/base_vector.h>

#include "abstract_page_cache.h"
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
        typename VectorType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::VectorType,
        typename FrameType = typename ColumnTraits<DefType, groove_data::CollationMode::COLLATION_INDEXED>::FrameType>
    class IndexedColumn : public BaseColumn, public std::enable_shared_from_this<IndexedColumn<DefType>> {

    private:
        std::shared_ptr<AbstractPageCache> m_pageCache;

        IndexedColumn(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            std::shared_ptr<AbstractPageCache> pageCache)
            : BaseColumn(datasetUrl, modelId, columnId),
              m_pageCache(pageCache)
        {
        };

    public:

        /**
         *
         * @param key
         * @return
         */
        tempo_utils::Result<DatumType>
        getValue(KeyType key)
        {
            DatumType result;
            result.fidelity = groove_data::DatumFidelity::FIDELITY_UNKNOWN;

            auto getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
                getDatasetUrl(), getModelId(), getColumnId(), Option<KeyType>(key), false);
            if (getIndexedPageResult.isStatus()) {
                auto status = getIndexedPageResult.getStatus();
                if (status.matchesCondition(ModelCondition::kPageNotFound))
                    return result;
                return status;
            }
            auto page = getIndexedPageResult.getResult();
            page->getDatum(key, result);
            return result;
        };

        /**
         *
         * @param key
         * @return
         */
        groove_data::DatumFidelity
        getFidelity(KeyType key)
        {
            auto getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
                getDatasetUrl(), getModelId(), getColumnId(), Option<KeyType>(key), false);
            if (getIndexedPageResult.isStatus())
                return groove_data::DatumFidelity::FIDELITY_UNKNOWN;
            DatumType result;
            auto page = getIndexedPageResult.getResult();
            if (!page->getDatum(key, result))
                return groove_data::DatumFidelity::FIDELITY_UNKNOWN;
            return result.fidelity;
        };

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

            auto getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
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
                getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
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
         * @param range
         * @return
         */
        tempo_utils::Result<std::vector<std::shared_ptr<VectorType>>>
        getVectors(const RangeType &range)
        {
            std::vector<std::shared_ptr<VectorType>> vectors;
            Option<KeyType> key = range.start;

            auto getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
                getDatasetUrl(), getModelId(), getColumnId(), key, false);
            if (getIndexedPageResult.isStatus()) {
                auto status = getIndexedPageResult.getStatus();
                if (!status.matchesCondition(ModelCondition::kPageNotFound))
                    return status;
                return vectors;
            }
            auto page = getIndexedPageResult.getResult();

            auto vector = page->getVector();
            auto slice = vector->slice(range);
            vectors.push_back(slice);

            auto largestOption = slice->getLargest();
            while (!largestOption.isEmpty()) {
                auto largest = largestOption.getValue();
                getIndexedPageResult = m_pageCache->template getIndexedPage<DefType>(
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
                vectors.push_back(slice);
                largestOption = slice->getLargest();
            }

            return vectors;
        }

        /**
         *
         * @param range
         * @return
         */
        tempo_utils::Result<std::vector<std::shared_ptr<FrameType>>>
        getFrames(const RangeType &range)
        {
            auto getVectorsResult = getVectors(range);
            if (getVectorsResult.isStatus())
                return getVectorsResult.getStatus();
            auto vectors = getVectorsResult.getResult();
            std::vector<std::shared_ptr<FrameType>> frames;
            for (const auto &vector : vectors) {
                auto createFrameResult = FrameType::create(
                    vector->getTable(),
                    vector->getKeyFieldIndex(),
                    {{vector->getValFieldIndex(), vector->getFidFieldIndex()}});
                if (createFrameResult.isStatus())
                    return createFrameResult.getStatus();
                frames.push_back(createFrameResult.getResult());
            }
            return frames;
        }

        /**
         *
         * @param modelId
         * @param columnId
         * @param store
         * @return
         */
        static std::shared_ptr<IndexedColumn<DefType>>
        create(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId,
            std::shared_ptr<AbstractPageCache> pageCache)
        {
            return std::shared_ptr<IndexedColumn<DefType>>(
                new IndexedColumn<DefType>(datasetUrl, modelId, columnId, pageCache));
        };
    };
}

#endif // GROOVE_MODEL_INDEXED_COLUMN_TEMPLATE_H