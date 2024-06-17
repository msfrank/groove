#ifndef GROOVE_IO_IO_RESULT_H
#define GROOVE_IO_IO_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_io {

    constexpr tempo_utils::SchemaNs kGrooveIOStatusNs("dev.zuri.ns:groove-io-status-1");

    enum class IOCondition {
        kIOInvariant,
    };

    class IOStatus : public tempo_utils::TypedStatus<IOCondition> {
    public:
        using TypedStatus::TypedStatus;
        static IOStatus ok();
        static bool convert(IOStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        IOStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static IOStatus forCondition(
            IOCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return IOStatus(condition, message);
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
        static IOStatus forCondition(
            IOCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return IOStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_io::IOStatus> {
        using ConditionType = groove_io::IOCondition;
        static bool convert(groove_io::IOStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_io::IOStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_io::IOCondition> {
        using StatusType = groove_io::IOStatus;
        static constexpr const char *condition_namespace() { return groove_io::kGrooveIOStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_io::IOCondition condition)
        {
            switch (condition) {
                case groove_io::IOCondition::kIOInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_io::IOCondition condition)
        {
            switch (condition) {
                case groove_io::IOCondition::kIOInvariant:
                    return "IO invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_IO_IO_RESULT_H