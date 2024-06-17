
#include <groove_units/units_result.h>

groove_units::UnitsStatus::UnitsStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<UnitsCondition>(statusCode, detail)
{
}

groove_units::UnitsStatus
groove_units::UnitsStatus::ok()
{
    return UnitsStatus();
}

bool
groove_units::UnitsStatus::convert(UnitsStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kTempoUnitsStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = UnitsStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
