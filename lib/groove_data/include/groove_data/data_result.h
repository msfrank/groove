#ifndef GROOVE_DATA_DATA_RESULT_H
#define GROOVE_DATA_DATA_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_data {

    constexpr tempo_utils::SchemaNs kGrooveDataStatusNs("dev.zuri.ns:groove-data-status-1");

    enum class DataCondition {
        kDataInvariant,
    };

    class DataStatus : public tempo_utils::TypedStatus<DataCondition> {
    public:
        using TypedStatus::TypedStatus;
        static DataStatus ok();
        static bool convert(DataStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        DataStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static DataStatus forCondition(
            DataCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return DataStatus(condition, message);
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
        static DataStatus forCondition(
            DataCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return DataStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_data::DataStatus> {
        using ConditionType = groove_data::DataCondition;
        static bool convert(groove_data::DataStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_data::DataStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_data::DataCondition> {
        using StatusType = groove_data::DataStatus;
        static constexpr const char *condition_namespace() { return groove_data::kGrooveDataStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_data::DataCondition condition)
        {
            switch (condition) {
                case groove_data::DataCondition::kDataInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_data::DataCondition condition)
        {
            switch (condition) {
                case groove_data::DataCondition::kDataInvariant:
                    return "Data invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_DATA_DATA_RESULT_H