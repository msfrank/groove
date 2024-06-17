
#include <groove_shapes/shapes_result.h>

groove_shapes::ShapesStatus::ShapesStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ShapesCondition>(statusCode, detail)
{
}

groove_shapes::ShapesStatus
groove_shapes::ShapesStatus::ok()
{
    return ShapesStatus();
}

bool
groove_shapes::ShapesStatus::convert(ShapesStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveShapesStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = ShapesStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
