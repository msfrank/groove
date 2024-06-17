
#include <groove_agent/channel_monitor.h>
#include <tempo_utils/log_stream.h>

ChannelMonitor::ChannelMonitor(
    uv_loop_t *loop,
    grpc::channelz::v1::Channelz::Stub *stub,
    int idleTimeoutSeconds)
    : m_loop(loop),
      m_stub(stub),
      m_idleTimeoutSeconds(idleTimeoutSeconds)
{
    TU_ASSERT (m_loop != nullptr);
    TU_ASSERT (m_stub != nullptr);
}

static void
on_idle_timer(uv_timer_t *timer)
{
    auto *monitor = (ChannelMonitor *) timer->data;
    if (monitor->isIdle()) {
        TU_LOG_INFO << "idle timeout exceeded";
        uv_timer_stop(timer);
        uv_stop(timer->loop);
        uv_close((uv_handle_t*) timer, nullptr);
    }
}

tempo_utils::Status
ChannelMonitor::ChannelMonitor::initialize()
{
    absl::MutexLock locker(&m_lock);

    uv_timer_init(m_loop, &m_idle);
    m_idle.data = this;
    if (m_idleTimeoutSeconds > 0) {
        uv_timer_start(&m_idle, on_idle_timer, 1000, 1000); // run timer cb every 1000 ms
    }
    m_lastActivity = absl::Now();

    return groove_storage::StorageStatus::ok();
}

static tempo_utils::Result<absl::Time>
update_last_activity(absl::Time lastActivity, grpc::channelz::v1::Channelz::Stub *stub)
{
    grpc::ClientContext getServersCtx;
    grpc::channelz::v1::GetServersRequest getServersRequest;
    getServersRequest.set_start_server_id(0);
    getServersRequest.set_max_results(10);
    grpc::channelz::v1::GetServersResponse getServersResponse;

    grpc::Status status = stub->GetServers(&getServersCtx, getServersRequest, &getServersResponse);
    if (!status.ok())
        return tempo_utils::GenericStatus::forCondition(
            tempo_utils::GenericCondition::kInternalViolation, status.error_message());

    //TU_LOG_INFO << getServersResponse.server_size() << " servers found";
    TU_LOG_ERROR_IF(!getServersResponse.end()) << "expected less than 10 servers";

    for (int serverIndex = 0; serverIndex < getServersResponse.server_size(); serverIndex++) {
        const auto &server = getServersResponse.server(serverIndex);

        tu_int64 startSocketId = 0;
        do {
            grpc::ClientContext getServerSocketsCtx;
            grpc::channelz::v1::GetServerSocketsRequest getServerSocketsRequest;
            getServerSocketsRequest.set_server_id(server.ref().server_id());
            getServerSocketsRequest.set_start_socket_id(startSocketId);
            getServerSocketsRequest.set_max_results(1024);
            grpc::channelz::v1::GetServerSocketsResponse getServerSocketsResponse;

            status = stub->GetServerSockets(&getServerSocketsCtx, getServerSocketsRequest, &getServerSocketsResponse);
            if (!status.ok())
                return tempo_utils::GenericStatus::forCondition(
                    tempo_utils::GenericCondition::kInternalViolation, status.error_message());

            //TU_LOG_INFO << getServerSocketsResponse.socket_ref_size()
            //    << " server sockets found for " << server.ref().name();
            if (getServerSocketsResponse.socket_ref_size() == 0 && getServerSocketsResponse.end())
                break;

            for (int socketIndex = 0; socketIndex < getServerSocketsResponse.socket_ref_size(); socketIndex++) {
                const auto &socketRef = getServerSocketsResponse.socket_ref(socketIndex);

                grpc::ClientContext getSocketCtx;
                grpc::channelz::v1::GetSocketRequest getSocketRequest;
                getSocketRequest.set_socket_id(socketRef.socket_id());
                getSocketRequest.set_summary(true);
                grpc::channelz::v1::GetSocketResponse getSocketResponse;

                status = stub->GetSocket(&getSocketCtx, getSocketRequest, &getSocketResponse);
                if (!status.ok())
                    return tempo_utils::GenericStatus::forCondition(
                        tempo_utils::GenericCondition::kInternalViolation, status.error_message());

                const auto &socketData = getSocketResponse.socket().data();

                auto finishedStreams = socketData.streams_succeeded() + socketData.streams_failed();
                auto activeStreams = socketData.streams_started() - finishedStreams;
                auto lastReceivedAt = absl::FromUnixSeconds(socketData.last_message_received_timestamp().seconds());

                TU_LOG_INFO << server.ref().name()
                    << " has active streams: " << activeStreams
                    << ", finished streams: " << finishedStreams
                    << ", message last received at " << absl::FormatTime(lastReceivedAt);

                if (activeStreams > 0 && lastReceivedAt > lastActivity) {
                    lastActivity = lastReceivedAt;
                }

                if (socketRef.socket_id() > startSocketId) {
                    startSocketId = socketRef.socket_id();
                }
            }

            //
            if (getServerSocketsResponse.end()) {
                startSocketId = -1;
            }

        } while (startSocketId > 0);
    }

    return lastActivity;
}

bool
ChannelMonitor::ChannelMonitor::isIdle()
{
    auto getLastActivityResult = update_last_activity(m_lastActivity, m_stub);
    if (getLastActivityResult.isStatus()) {
        TU_LOG_ERROR << "channel monitor failed to get activity status: " << getLastActivityResult.getStatus();
        return true;
    }
    m_lastActivity = getLastActivityResult.getResult();
    auto durationSinceLastActivity = absl::Now() - m_lastActivity;
    auto secondsSinceLastActivity = ToInt64Seconds(durationSinceLastActivity);
    TU_LOG_VV << "check idle: " << secondsSinceLastActivity << " seconds since last activity";
    return secondsSinceLastActivity >= m_idleTimeoutSeconds;
}
