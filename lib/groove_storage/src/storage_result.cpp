
#include <groove_storage/storage_result.h>

groove_storage::StorageStatus::StorageStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<StorageCondition>(statusCode, detail)
{
}

groove_storage::StorageStatus
groove_storage::StorageStatus::ok()
{
    return StorageStatus();
}

bool
groove_storage::StorageStatus::convert(StorageStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kGrooveStorageStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = StorageStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
