
#include <groove_data/data_result.h>

groove_data::DataStatus::DataStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<DataCondition>(statusCode, detail)
{
}

groove_data::DataStatus
groove_data::DataStatus::ok()
{
    return DataStatus();
}

bool
groove_data::DataStatus::convert(DataStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveDataStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = DataStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}