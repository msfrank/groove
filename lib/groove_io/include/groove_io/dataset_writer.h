#ifndef GROOVE_IO_DATASET_WRITER_H
#define GROOVE_IO_DATASET_WRITER_H

#include <filesystem>

#include <absl/container/btree_map.h>

#include <groove_data/base_frame.h>
#include <groove_io/generated/index.h>
#include <groove_model/groove_schema.h>
#include <groove_model/page_id.h>

#include "io_result.h"

namespace groove_io {

    struct DatasetWriterOptions {
        int maxFrameSize = 1024 * 1024;     // if frame is larger than maxFrameSize then split it into multiple frames
        int idealFrameSize = 65536;         // when splitting frames use idealFrameSize as the target size
        bool stripMetadata = true;          // if true then remove custom key-value metadata from frames
    };

    class DatasetWriter {
    public:
        DatasetWriter(const tempo_utils::Url &datasetUrl, const groove_model::GrooveSchema &schema);
        DatasetWriter(
            const tempo_utils::Url &datasetUrl,
            const groove_model::GrooveSchema &schema,
            const DatasetWriterOptions &options);

        tempo_utils::Status putVector(const std::string &modelId, std::shared_ptr<groove_data::BaseVector> vector);
        tempo_utils::Status putFrame(const std::string &modelId, std::shared_ptr<groove_data::BaseFrame> frame);

        tempo_utils::Status writeDataset(const std::filesystem::path &datasetPath);

    private:
        tempo_utils::Url m_datasetUrl;
        groove_model::GrooveSchema m_schema;
        DatasetWriterOptions m_options;

        struct VectorPriv {
            groove_model::PageId pageId;
            tu_uint32 frameIndex;
            tu_uint32 valOffset;
            tu_uint32 fidOffset;
        };
        std::vector<VectorPriv> m_vectors;

        struct FramePriv {
            tu_uint32 keyOffset;
            std::shared_ptr<const arrow::Buffer> frameBytes;
        };
        std::vector<std::unique_ptr<FramePriv>> m_frames;

        absl::btree_map<
            groove_model::PageId,
            std::pair<uint32_t,groove_model::PageId>> m_pageIdToOffset;

        tempo_utils::Status putVector(
            const std::string &modelId,
            std::shared_ptr<groove_data::BaseVector> vector,
            tu_uint32 frameIndex);
    };
}

#endif // GROOVE_IO_DATASET_WRITER_H