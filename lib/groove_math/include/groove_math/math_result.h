#ifndef GROOVE_MATH_MATH_RESULT_H
#define GROOVE_MATH_MATH_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_math {

    constexpr tempo_utils::SchemaNs kGrooveMathStatusNs("dev.zuri.ns:groove-math-status-1");

    enum class MathCondition {
        kMathInvariant,
    };

    class MathStatus : public tempo_utils::TypedStatus<MathCondition> {
    public:
        using TypedStatus::TypedStatus;
        static MathStatus ok();
        static bool convert(MathStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        MathStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static MathStatus forCondition(
            MathCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return MathStatus(condition, message);
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
        static MathStatus forCondition(
            MathCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return MathStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_math::MathStatus> {
        using ConditionType = groove_math::MathCondition;
        static bool convert(groove_math::MathStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_math::MathStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_math::MathCondition> {
        using StatusType = groove_math::MathStatus;
        static constexpr const char *condition_namespace() { return groove_math::kGrooveMathStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_math::MathCondition condition)
        {
            switch (condition) {
                case groove_math::MathCondition::kMathInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_math::MathCondition condition)
        {
            switch (condition) {
                case groove_math::MathCondition::kMathInvariant:
                    return "Math invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_MATH_MATH_RESULT_H
