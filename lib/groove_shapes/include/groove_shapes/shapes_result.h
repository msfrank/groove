#ifndef GROOVE_SHAPES_SHAPES_RESULT_H
#define GROOVE_SHAPES_SHAPES_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_shapes {

    constexpr tempo_utils::SchemaNs kGrooveShapesStatusNs("dev.zuri.ns:groove-shapes-status-1");

    enum class ShapesCondition {
        kInvalidSource,
        kMissingItem,
        kShapesInvariant,
    };

    class ShapesStatus : public tempo_utils::TypedStatus<ShapesCondition> {
    public:
        using TypedStatus::TypedStatus;
        static ShapesStatus ok();
        static bool convert(ShapesStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ShapesStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ShapesStatus forCondition(
            ShapesCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ShapesStatus(condition, message);
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
        static ShapesStatus forCondition(
            ShapesCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ShapesStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_shapes::ShapesStatus> {
        using ConditionType = groove_shapes::ShapesCondition;
        static bool convert(groove_shapes::ShapesStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_shapes::ShapesStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_shapes::ShapesCondition> {
        using StatusType = groove_shapes::ShapesStatus;
        static constexpr const char *condition_namespace() { return groove_shapes::kGrooveShapesStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_shapes::ShapesCondition condition)
        {
            switch (condition) {
                case groove_shapes::ShapesCondition::kInvalidSource:
                    return StatusCode::kInvalidArgument;
                case groove_shapes::ShapesCondition::kMissingItem:
                    return StatusCode::kNotFound;
                case groove_shapes::ShapesCondition::kShapesInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_shapes::ShapesCondition condition)
        {
            switch (condition) {
                case groove_shapes::ShapesCondition::kInvalidSource:
                    return "Invalid source";
                case groove_shapes::ShapesCondition::kMissingItem:
                    return "Missing item";
                case groove_shapes::ShapesCondition::kShapesInvariant:
                    return "Shapes invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_SHAPES_SHAPES_RESULT_H
