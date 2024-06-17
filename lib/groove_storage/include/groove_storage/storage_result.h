#ifndef GROOVE_STORAGE_STORAGE_RESULT_H
#define GROOVE_STORAGE_STORAGE_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_storage {

    constexpr tempo_utils::SchemaNs kGrooveStorageStatusNs("dev.zuri.ns:groove-storage-status-1");

    enum class StorageCondition {
        kStorageInvariant,
    };

    class StorageStatus : public tempo_utils::TypedStatus<StorageCondition> {
    public:
        using TypedStatus::TypedStatus;
        static StorageStatus ok();
        static bool convert(StorageStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        StorageStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template<typename... Args>
        static StorageStatus forCondition(
            StorageCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return StorageStatus(condition, message);
        }

        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template<typename... Args>
        static StorageStatus forCondition(
            StorageCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return StorageStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_storage::StorageStatus> {
        using ConditionType = groove_storage::StorageCondition;
        static bool convert(groove_storage::StorageStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_storage::StorageStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_storage::StorageCondition> {
        using StatusType = groove_storage::StorageStatus;
        static constexpr const char *condition_namespace() { return groove_storage::kGrooveStorageStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_storage::StorageCondition condition)
        {
            switch (condition) {
                case groove_storage::StorageCondition::kStorageInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_storage::StorageCondition condition)
        {
            switch (condition) {
                case groove_storage::StorageCondition::kStorageInvariant:
                    return "Storage invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_STORAGE_STORAGE_RESULT_H
