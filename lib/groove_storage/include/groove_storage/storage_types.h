#ifndef GROOVE_STORAGE_STORAGE_TYPES_H
#define GROOVE_STORAGE_STORAGE_TYPES_H

#include <groove_data/data_types.h>
#include <tempo_utils/url.h>

namespace groove_storage {

    enum class ShardVersion {
        Unknown,
        Version1,
    };

    enum class IntervalType {
        Invalid,
        Category,
        Double,
        Int64,
    };

    enum class DataProviderState {
        INITIAL,
        UNCONNECTED,
        CONNECTING,
        CONNECTED,
        SHUTTING_DOWN,
        FAILED
    };

    enum class DataStreamState {
        INITIAL,
        REQUEST_AUTH,
        PENDING_AUTH,
        RUNNING,
        FAILED,
        FINISHED
    };

    enum class DataExpressionState {
        INITIAL,
        RUNNING,
        FAILED,
        FINISHED
    };

    class BaseSyncRequest {
    public:
        enum class SyncRequestType {
            CATEGORY_REQUEST,
            DOUBLE_REQUEST,
            INT64_REQUEST,
        };

        SyncRequestType getType() const;
        tempo_utils::Url getDatasetUrl() const;
        std::string getModelId() const;

    protected:
        BaseSyncRequest(
            SyncRequestType type,
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId);

    private:
        SyncRequestType m_type;
        tempo_utils::Url m_datasetUrl;
        std::string m_modelId;
    };

    class CategorySyncRequest : public BaseSyncRequest {
    public:
        CategorySyncRequest(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::CategoryRange &range);

        groove_data::CategoryRange getRange() const;

    private:
        groove_data::CategoryRange m_range;
    };

    class DoubleSyncRequest : public BaseSyncRequest {
    public:
        DoubleSyncRequest(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::DoubleRange &range);

        groove_data::DoubleRange getRange() const;

    private:
        groove_data::DoubleRange m_range;
    };

    class Int64SyncRequest : public BaseSyncRequest {
    public:
        Int64SyncRequest(
            const tempo_utils::Url &datasetUrl,
            const std::string &modelId,
            const groove_data::Int64Range &range);

        groove_data::Int64Range getRange() const;

    private:
        groove_data::Int64Range m_range;
    };

    // forward declarations
    namespace internal {
        class ShardReader;
    }
}

#endif // GROOVE_STORAGE_STORAGE_TYPES_H