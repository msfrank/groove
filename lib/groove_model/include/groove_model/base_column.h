#ifndef GROOVE_MODEL_BASE_COLUMN_H
#define GROOVE_MODEL_BASE_COLUMN_H

#include <string>

#include <groove_data/data_types.h>

#include "abstract_page_cache.h"

namespace groove_model {

    class BaseColumn {

    public:
        BaseColumn(
            const tempo_utils::Url &datasetUrl,
            std::shared_ptr<const std::string> modelId,
            std::shared_ptr<const std::string> columnId);
        virtual ~BaseColumn() = default;

        tempo_utils::Url getDatasetUrl() const;
        std::shared_ptr<const std::string> getModelId() const;
        std::shared_ptr<const std::string> getColumnId() const;

    private:
        tempo_utils::Url m_datasetUrl;
        std::shared_ptr<const std::string> m_modelId;
        std::shared_ptr<const std::string> m_columnId;
    };
}

#endif // GROOVE_MODEL_BASE_COLUMN_H