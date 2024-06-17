#ifndef GROOVE_MODEL_MODEL_RESULT_H
#define GROOVE_MODEL_MODEL_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace groove_model {

    constexpr tempo_utils::SchemaNs kGrooveModelStatusNs("dev.zuri.ns:groove-model-status-1");

    enum class ModelCondition {
        kColumnNotFound,
        kPageNotFound,
        kModelInvariant,
    };

    class ModelStatus : public tempo_utils::TypedStatus<ModelCondition> {
    public:
        using TypedStatus::TypedStatus;
        static ModelStatus ok();
        static bool convert(ModelStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ModelStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ModelStatus forCondition(
            ModelCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ModelStatus(condition, message);
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
        static ModelStatus forCondition(
            ModelCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs) {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ModelStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<groove_model::ModelStatus> {
        using ConditionType = groove_model::ModelCondition;
        static bool convert(groove_model::ModelStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return groove_model::ModelStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<groove_model::ModelCondition> {
        using StatusType = groove_model::ModelStatus;
        static constexpr const char *condition_namespace() { return groove_model::kGrooveModelStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(groove_model::ModelCondition condition)
        {
            switch (condition) {
                case groove_model::ModelCondition::kColumnNotFound:
                    return StatusCode::kNotFound;
                case groove_model::ModelCondition::kPageNotFound:
                    return StatusCode::kNotFound;
                case groove_model::ModelCondition::kModelInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(groove_model::ModelCondition condition)
        {
            switch (condition) {
                case groove_model::ModelCondition::kColumnNotFound:
                    return "Column not found";
                case groove_model::ModelCondition::kPageNotFound:
                    return "Page not found";
                case groove_model::ModelCondition::kModelInvariant:
                    return "Model invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // GROOVE_MODEL_MODEL_RESULT_H
