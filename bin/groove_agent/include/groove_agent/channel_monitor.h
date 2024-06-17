#ifndef GROOVE_AGENT_CHANNEL_MONITOR_H
#define GROOVE_AGENT_CHANNEL_MONITOR_H

#include <uv.h>

#include <gens/src/proto/grpc/channelz/channelz.grpc.pb.h>
#include <gens/src/proto/grpc/channelz/channelz.pb.h>

#include <groove_storage/storage_result.h>

class ChannelMonitor {

public:
    ChannelMonitor(uv_loop_t *loop, grpc::channelz::v1::Channelz::Stub *stub, int idleTimeoutSeconds);

    tempo_utils::Status initialize();
    bool isIdle();

private:
    uv_loop_t *m_loop;
    grpc::channelz::v1::Channelz::Stub *m_stub;
    int m_idleTimeoutSeconds;
    absl::Mutex m_lock;
    uv_timer_t m_idle;
    absl::Time m_lastActivity;
};

#endif // GROOVE_AGENT_CHANNEL_MONITOR_H