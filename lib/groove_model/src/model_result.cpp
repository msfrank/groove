
#include <groove_model/model_result.h>

groove_model::ModelStatus::ModelStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ModelCondition>(statusCode, detail)
{
}

groove_model::ModelStatus
groove_model::ModelStatus::ok()
{
    return ModelStatus();
}

bool
groove_model::ModelStatus::convert(ModelStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveModelStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = ModelStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
