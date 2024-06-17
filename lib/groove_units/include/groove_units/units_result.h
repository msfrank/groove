#ifndef GROOVE_UNITS_UNITS_RESULT_H
#define GROOVE_UNITS_UNITS_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_units {

    constexpr tempo_utils::SchemaNs kTempoUnitsStatusNs("dev.zuri.ns:tempo-units-status-1");

    enum class UnitsCondition {
        kMissingBaseUnit,
        kMissingScaleProduct,
        kUnitDimensionConflict,
        kUnitExponentConflict,
        kScaleProductBaseConflict,
        kUnitsInvariant,
    };

    class UnitsStatus : public tempo_utils::TypedStatus<UnitsCondition> {
    public:
        using TypedStatus::TypedStatus;
        static UnitsStatus ok();
        static bool convert(UnitsStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        UnitsStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static UnitsStatus forCondition(
            UnitsCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return UnitsStatus(condition, message);
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
        static UnitsStatus forCondition(
            UnitsCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return UnitsStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_units::UnitsStatus> {
        using ConditionType = groove_units::UnitsCondition;
        static bool convert(groove_units::UnitsStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_units::UnitsStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_units::UnitsCondition> {
        using StatusType = groove_units::UnitsStatus;
        static constexpr const char *condition_namespace() { return groove_units::kTempoUnitsStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_units::UnitsCondition condition)
        {
            switch (condition) {
                case groove_units::UnitsCondition::kMissingBaseUnit:
                    return StatusCode::kNotFound;
                case groove_units::UnitsCondition::kMissingScaleProduct:
                    return StatusCode::kNotFound;
                case groove_units::UnitsCondition::kUnitDimensionConflict:
                    return StatusCode::kInvalidArgument;
                case groove_units::UnitsCondition::kUnitExponentConflict:
                    return StatusCode::kInvalidArgument;
                case groove_units::UnitsCondition::kScaleProductBaseConflict:
                    return StatusCode::kInvalidArgument;
                case groove_units::UnitsCondition::kUnitsInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_units::UnitsCondition condition)
        {
            switch (condition) {
                case groove_units::UnitsCondition::kMissingBaseUnit:
                    return "Missing base unit";
                case groove_units::UnitsCondition::kMissingScaleProduct:
                    return "Missing scale product";
                case groove_units::UnitsCondition::kUnitDimensionConflict:
                    return "Unit dimension conflict";
                case groove_units::UnitsCondition::kUnitExponentConflict:
                    return "Unit exponent conflict";
                case groove_units::UnitsCondition::kScaleProductBaseConflict:
                    return "Scale product base conflict";
                case groove_units::UnitsCondition::kUnitsInvariant:
                    return "Units invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_UNITS_UNITS_RESULT_H
