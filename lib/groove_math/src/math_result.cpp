
#include <groove_math/math_result.h>

groove_math::MathStatus::MathStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<MathCondition>(statusCode, detail)
{
}

groove_math::MathStatus
groove_math::MathStatus::ok()
{
    return MathStatus();
}

bool
groove_math::MathStatus::convert(MathStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveMathStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = MathStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
