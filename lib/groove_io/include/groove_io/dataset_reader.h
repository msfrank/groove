#ifndef GROOVE_IO_GROOVE_DATASET_H
#define GROOVE_IO_GROOVE_DATASET_H

#include <filesystem>

#include <boost/interprocess/managed_mapped_file.hpp>

#include <arrow/io/file.h>

#include <groove_data/base_frame.h>
#include <groove_model/abstract_page_cache.h>
#include <groove_model/groove_model.h>
#include <groove_model/groove_schema.h>
#include <tempo_utils/slice.h>

#include "groove_index.h"
#include "io_result.h"

namespace groove_io {

    class DatasetReader : public groove_model::AbstractPageCache, public std::enable_shared_from_this<DatasetReader> {
    public:
        bool isValid() const;

        std::filesystem::path getDatasetPath() const;
        tu_uint8 getVersion() const;
        tu_uint8 getFlags() const;
        groove_model::GrooveSchema getSchema() const;

        bool isEmpty() override;
        tempo_utils::Result<groove_model::PageId> getPageIdBefore(
            const groove_model::PageId &pageId,
            bool exclusive) override;
        tempo_utils::Result<groove_model::PageId> getPageIdAfter(
            const groove_model::PageId &pageId,
            bool exclusive) override;
        tempo_utils::Result<std::shared_ptr<arrow::Buffer>> getPageData(
            const groove_model::PageId &pageId) override;
        tempo_utils::Status pageExists(const groove_model::PageId &pageId) override;

        static tempo_utils::Result<std::shared_ptr<DatasetReader>> create(const std::filesystem::path &path);

    private:
        std::filesystem::path m_datasetPath;
        boost::interprocess::mapped_region m_mappedRegion;
        tu_uint8 m_version;
        tu_uint8 m_flags;
        GrooveIndex m_index;
        groove_model::GrooveSchema m_schema;
        tempo_utils::Slice m_content;

        DatasetReader(
            const std::filesystem::path &datasetPath,
            tu_uint8 version,
            tu_uint8 flags,
            GrooveIndex index,
            groove_model::GrooveSchema schema,
            tempo_utils::Slice content);
    };

    class DatasetPage : public arrow::Buffer {
    public:
        DatasetPage(tempo_utils::Slice bytes);

    private:
        tempo_utils::Slice m_bytes;
    };
}

#endif // GROOVE_IO_GROOVE_DATASET_H