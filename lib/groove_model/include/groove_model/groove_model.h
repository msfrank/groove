#ifndef GROOVE_MODEL_GROOVE_MODEL_H
#define GROOVE_MODEL_GROOVE_MODEL_H

#include <filesystem>
#include <string>

#include <absl/container/flat_hash_map.h>

#include <groove_data/base_frame.h>
#include <tempo_utils/url.h>

#include "base_column.h"
#include "groove_schema.h"
#include "indexed_column_template.h"
#include "persistent_caching_page_store.h"
#include "rocksdb_store.h"

namespace groove_model {

    class ColumnDef {

    public:
        ColumnDef();
        ColumnDef(
            groove_data::CollationMode collation,
            groove_data::DataKeyType key,
            groove_data::DataValueType value);
        ColumnDef(const ColumnDef &other);

        bool isValid() const;
        groove_data::CollationMode getCollation() const;
        groove_data::DataKeyType getKey() const;
        groove_data::DataValueType getValue() const;

    private:
        groove_data::CollationMode m_collation;
        groove_data::DataKeyType m_key;
        groove_data::DataValueType m_value;
    };

    class GrooveModel {

    public:
        GrooveModel(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            groove_data::CollationMode collation,
            groove_data::DataKeyType key,
            const absl::flat_hash_map<std::string,groove_data::DataValueType> &columns,
            std::shared_ptr<AbstractPageCache> pageCache);

        tempo_utils::Url getDatasetUrl() const;
        std::shared_ptr<const std::string> getModelId() const;
        groove_data::CollationMode getCollationMode() const;
        groove_data::DataKeyType getKeyType();

        bool hasColumn(const std::string &columnId) const;
        bool hasColumn(const std::string &columnId, groove_data::DataValueType valueType) const;
        bool hasColumn(const std::string &columnId, const ColumnDef &columnDef) const;
        ColumnDef getColumnDef(const std::string &columnId) const;
        absl::flat_hash_map<std::string,ColumnDef>::const_iterator columnsBegin() const;
        absl::flat_hash_map<std::string,ColumnDef>::const_iterator columnsEnd() const;

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<const std::string> m_modelId;
        groove_data::CollationMode m_collation;
        groove_data::DataKeyType m_key;
        absl::flat_hash_map<std::string,ColumnDef> m_columns;
        std::shared_ptr<AbstractPageCache> m_pageCache;

    public:

        /**
         *
         * @tparam DefType
         * @tparam KeyType
         * @param columnId
         * @return
         */
        template <typename DefType,
            typename KeyType = typename DefType::KeyType>
        tempo_utils::Result<std::shared_ptr<IndexedColumn<DefType>>>
        getIndexedColumn(const std::string &columnId)
        {
            if (!m_columns.template contains(columnId))
                return ModelStatus::forCondition(ModelCondition::kColumnNotFound);
            auto columnDef = m_columns.template at(columnId);
            if (columnDef.getCollation() != groove_data::CollationMode::COLLATION_INDEXED)
                return ModelStatus::forCondition(ModelCondition::kColumnNotFound);
            if (columnDef.getKey() != DefType::static_key_type())
                return ModelStatus::forCondition(ModelCondition::kColumnNotFound);
            if (columnDef.getValue() != DefType::static_value_type())
                return ModelStatus::forCondition(ModelCondition::kColumnNotFound);
            return IndexedColumn<DefType>::create(
                m_datasetUrl, m_modelId, std::make_shared<const std::string>(columnId), m_pageCache);
        };
    };
}

#endif // GROOVE_MODEL_GROOVE_MODEL_H