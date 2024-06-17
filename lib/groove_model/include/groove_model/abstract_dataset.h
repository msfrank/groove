#ifndef GROOVE_MODEL_ABSTRACT_DATASET_H
#define GROOVE_MODEL_ABSTRACT_DATASET_H

#include <tempo_utils/url.h>

#include "groove_model.h"
#include "groove_schema.h"

namespace groove_model {

    class AbstractDataset {
    public:
        virtual ~AbstractDataset() = default;

        virtual tempo_utils::Url getDatasetUrl() const = 0;
        virtual groove_model::GrooveSchema getSchema() const = 0;
        virtual bool isImmutable() const = 0;
        virtual bool hasModel(const std::string &modelId) const = 0;
        virtual std::shared_ptr<groove_model::GrooveModel> getModel(const std::string &modelId) const = 0;
    };
}

#endif // GROOVE_MODEL_ABSTRACT_DATASET_H
