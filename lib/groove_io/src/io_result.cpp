
#include <groove_io/io_result.h>

groove_io::IOStatus::IOStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<IOCondition>(statusCode, detail)
{
}

groove_io::IOStatus
groove_io::IOStatus::ok()
{
    return IOStatus();
}

bool
groove_io::IOStatus::convert(IOStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveIOStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = IOStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
