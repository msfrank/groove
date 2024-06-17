#ifndef GROOVE_MODEL_INDEXED_VARIANT_COLUMN_H
#define GROOVE_MODEL_INDEXED_VARIANT_COLUMN_H

#include <type_traits>

#include <groove_iterator/map_iterator_template.h>
#include <tempo_utils/iterator_template.h>

#include "column_traits.h"
#include "indexed_column_template.h"
#include "model_types.h"
#include "persistent_caching_page_store.h"
#include "variant_value.h"

namespace groove_model {

    template <typename KeyType>
    struct VariantValueDatum {
        KeyType key;
        VariantValue value;
        groove_data::DatumFidelity fidelity;
        VariantValueDatum(): key(), value(), fidelity(groove_data::DatumFidelity::INVALID) {};
        VariantValueDatum(KeyType key, VariantValue value, groove_data::DatumFidelity fidelity)
            : key(key), value(value), fidelity(fidelity) {};
        VariantValueDatum(const VariantValueDatum<KeyType> &other)
            : key(other.key), value(other.value), fidelity(other.fidelity) {};
    };

    /**
     *
     * @tparam KeyType
     * @tparam DatumType
     * @param datum
     * @return
     */
    template <typename KeyType, typename DatumType>
    VariantValueDatum<KeyType> to_variant_value(const DatumType &datum)
    {
        return VariantValueDatum<KeyType>(datum.key, VariantValue(datum.value), datum.fidelity);
    }

    /**
     *
     * @tparam KeyType
     */
    template <typename KeyType>
    class VariantValueDatumIterator : public Iterator<VariantValueDatum<KeyType>> {
    public:
        VariantValueDatumIterator() : m_input() {};
        VariantValueDatumIterator(std::shared_ptr<Iterator<VariantValueDatum<KeyType>>> input) : m_input(input) {};
        bool getNext(VariantValueDatum<KeyType> &datum) override {
            if (m_input == nullptr)
                return false;
            return m_input->getNext(datum);
        };
    private:
        std::shared_ptr<Iterator<VariantValueDatum<KeyType>>> m_input;
    };

    /**
     *
     * @tparam KeyType
     * @tparam RangeType
     */
    template <typename KeyType, typename RangeType>
    class AbstractIndexedVariantColumn {
    public:
        virtual ~AbstractIndexedVariantColumn() = default;
        virtual groove_data::DataValueType getValueType() const = 0;
        virtual tempo_utils::Result<VariantValueDatum<KeyType>> getValue(KeyType key) = 0;
        virtual tempo_utils::Result<groove_data::DatumFidelity> getFidelity(KeyType key) = 0;
        virtual tempo_utils::Result<VariantValueDatumIterator<KeyType>> getValues(const RangeType &range) = 0;
    };

    /**
     *
     * @tparam DefType
     * @tparam KeyType
     * @tparam RangeType
     */
    template <typename DefType,
        typename KeyType = typename DefType::KeyType,
        typename DatumType = typename ColumnTraits<DefType,groove_data::CollationMode::COLLATION_INDEXED>::DatumType,
        typename IteratorType = typename ColumnTraits<DefType,groove_data::CollationMode::COLLATION_INDEXED>::IteratorType,
        typename RangeType = typename ColumnTraits<DefType,groove_data::CollationMode::COLLATION_INDEXED>::RangeType>
    class IndexedVariantColumn:
        public AbstractIndexedVariantColumn<KeyType,RangeType>,
        public std::enable_shared_from_this<IndexedVariantColumn<DefType>> {

    private:
        std::shared_ptr<IndexedColumn<DefType>> m_column;

        IndexedVariantColumn(std::shared_ptr<IndexedColumn<DefType>> column) : m_column(column) {};

    public:

        groove_data::DataValueType getValueType() const override
        {
            return DefType::static_value_type();
        }

        /**
         *
         * @param key
         * @return
         */
        tempo_utils::Result<VariantValueDatum<KeyType>>
        getValue(KeyType key) override
        {
            auto result = m_column->getValue(key);
            if (result.isStatus())
                return result.getStatus();
            auto datum = result.getResult();
            return VariantValueDatum<KeyType>(key, VariantValue(datum.value), datum.fidelity);
        };

        /**
         *
         * @param key
         * @return
         */
        tempo_utils::Result<groove_data::DatumFidelity>
        getFidelity(KeyType key) override
        {
            auto result = m_column->getValue(key);
            if (result.isStatus())
                return result.getStatus();
            auto datum = result.getResult();
            return datum.fidelity;
        };

        /**
         *
         * @param range
         * @return
         */
        tempo_utils::Result<VariantValueDatumIterator<KeyType>>
        getValues(const RangeType &range) override
        {
            auto getValuesResult = m_column->getValues(range);
            if (getValuesResult.isStatus())
                return getValuesResult.getStatus();
            auto it = new groove_iterator::MapIterator<
                IteratorType,
                DatumType,
                VariantValueDatum<KeyType>>(getValuesResult.getResult(), to_variant_value);
            auto input = std::shared_ptr<groove_iterator::MapIterator<
                IteratorType,
                DatumType,
                VariantValueDatum<KeyType>>>(it);
            return VariantValueDatumIterator<KeyType>(input);
        };

        /**
         *
         * @param modelId
         * @param columnId
         * @param store
         * @return
         */
        static std::shared_ptr<IndexedVariantColumn<DefType>>
        create(std::shared_ptr<IndexedColumn<DefType>> column)
        {
            return std::shared_ptr<IndexedVariantColumn<DefType>>(new IndexedVariantColumn<DefType>(column));
        };
    };
}

#endif // GROOVE_MODEL_INDEXED_VARIANT_COLUMN_H