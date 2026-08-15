#include "florid/detail/ArmImpl.hpp"
#include "florid/Exceptions.hpp"
#include "florid/detail/tick.hpp"

#include <cstring>
#include <chrono>

namespace florid {

// ────────────────────────────────────────────────────────
//  ArmControl
// ────────────────────────────────────────────────────────

Duration ArmControl::firmwarePeriod() const {
    return Duration::fromUSec(m_impl ? m_impl->m_fw_dt_us : 4000);
}

Duration ArmControl::stateAge() const {
    if (!m_impl) return Duration::fromMSec(0);
    return Duration::fromUSec(m_impl->m_latency.stateAgeUs(detail::s_nowUs()));
}

Duration ArmControl::estimatedLatency() const {
    if (!m_impl) return Duration::fromMSec(0);
    return Duration::fromUSec(
        static_cast<std::uint64_t>(m_impl->m_latency.estimatedLatencyMs() * 1000.0));
}

double ArmControl::receiveJitterUs() const {
    if (!m_impl) return 0.0;
    return m_impl->m_latency.receiveJitterUs();
}

double ArmControl::receiveHz() const {
    if (!m_impl) return 0.0;
    return m_impl->m_latency.receiveHz(detail::s_nowUs());
}

bool ArmControl::isReconnecting() const {
    return m_impl ? m_impl->m_reconnecting.load() : false;
}

void ArmControl::finishMotion() {
    if (m_impl) m_impl->m_stop_flag = true;
}

void ArmControl::stopControl() {
    if (m_impl) {
        m_impl->m_stop_flag = true;
        m_impl->m_running = false;
    }
}

// ────────────────────────────────────────────────────────
//  ArmImpl
// ────────────────────────────────────────────────────────

ArmImpl::ArmImpl(std::unique_ptr<Transport> s_transport)
    : m_transport(std::move(s_transport))
{
    m_arm_control.m_impl = this;

    m_session.on_send([this](const std::uint8_t* s_data, std::size_t s_size) {
        m_transport->send(s_data, s_size);
    });

    m_transport->setReceiveCallback(s_onPhysData, this);

    s_fetchDeviceInfo();

    // ── Lifecycle: notify firmware SDK is connected ──
    {
        fci::arm::SdkClientConnectedRequestPacket s_req{};
        s_req.payload.dummy = 0;
        m_session.request(s_req, 50);
    }

    m_connected = true;
}

ArmImpl::~ArmImpl() {
    m_running = false;

    // ── Lifecycle: notify firmware SDK disconnected (best-effort) ──
    {
        fci::arm::SdkClientDisconnectedRequestPacket s_req{};
        s_req.payload.dummy = 0;
        m_session.request(s_req, 20);
    }
}

void ArmImpl::s_onPhysData(void* s_context, const std::uint8_t* s_data, std::size_t s_size) {
    auto* s_self = static_cast<ArmImpl*>(s_context);
    s_self->s_feedBytes(s_data, s_size);
}

void ArmImpl::s_feedBytes(const std::uint8_t* s_data, std::size_t s_size) {
    auto s_result = m_session.receive(s_data, s_size);
    if (!s_result) return;

    auto s_status = m_session.deserializer().get<fci::arm::ArmStatus>();
    if (s_status.seq != m_last_status_seq) {
        m_last_status_seq = s_status.seq;
        m_latency.markReceived(s_status.last_sdk_timestamp_us, detail::s_nowUs());
        if (!m_rx_queue.enqueue(s_status)) return;
        m_data_ready.release();
    }
}

void ArmImpl::s_fetchDeviceInfo() {
    fci::arm::GetDeviceInfoRequestPacket s_req{};
    s_req.payload.dummy = 0;

    // Firmware sends GetDeviceInfoResponse directly without USBAck,
    // so use notify() + manual req_id + poll.
    s_req.req_id = m_session.ack_manager().allocate();
    (void)m_session.notify(s_req);

    using namespace std::chrono;
    auto s_deadline = steady_clock::now() + milliseconds(100);

    while (steady_clock::now() < s_deadline) {
        auto s_response =
            m_session.deserializer().get<fci::arm::GetDeviceInfoResponsePacket>();
        if (s_response.req_id == s_req.req_id) {
            m_device_info = s_response.payload.info;
            return;
        }
        std::this_thread::yield();
    }

    throw ProtocolException(
        "GetDeviceInfo response not received within timeout");
}

ArmState ArmImpl::s_convertStatus(const fci::arm::ArmStatus& s_raw) {
    ArmState s_state;
    s_state.m_time = detail::get_tick_ms();
    s_state.m_seq = s_raw.seq;
    s_state.m_mode = static_cast<std::uint32_t>(static_cast<std::uint8_t>(s_raw.mode));
    s_state.m_source_timestamp_us = s_raw.timestamp_us;
    s_state.m_errors = s_raw.errors;

    for (int s_i = 0; s_i < 12; ++s_i) {
        s_state.m_q[s_i]   = s_raw.status.q[s_i];
        s_state.m_dq[s_i]  = s_raw.status.dq[s_i];
        s_state.m_tau[s_i] = s_raw.status.tau[s_i];
    }
    for (int s_i = 0; s_i < 6; ++s_i)
        s_state.m_base_gravity[s_i] = s_raw.base_gravity[s_i];
    for (int s_i = 0; s_i < 32; ++s_i)
        s_state.m_O_T_EE[s_i] = s_raw.O_T_EE[s_i];
    for (int s_i = 0; s_i < 12; ++s_i)
        s_state.m_F_ext[s_i] = s_raw.F_ext[s_i];

    for (int s_i = 0; s_i < 2; ++s_i) {
        s_state.m_gripper_q[s_i]   = s_raw.gripper.q[s_i];
        s_state.m_gripper_dq[s_i]  = s_raw.gripper.dq[s_i];
        s_state.m_gripper_tau[s_i] = s_raw.gripper.tau[s_i];
    }

    return s_state;
}

ArmState ArmImpl::readOnce() {
    fci::arm::ArmStatus s_raw;
    if (!m_rx_queue.try_dequeue(s_raw)) return ArmState{};
    return s_convertStatus(s_raw);
}

// ────────────────────────────────────────────────────────
//  Configuration commands
// ────────────────────────────────────────────────────────

void ArmImpl::s_requestPcMode() {
    fci::arm::SetArmModeRequestPacket s_req{};
    s_req.payload.mode = fci::arm::ArmMode::Pc;
    m_session.request(s_req, 200);
}

void ArmImpl::enable() {
    fci::arm::SetArmModeRequestPacket s_req{};
    s_req.payload.mode = fci::arm::ArmMode::Pc;
    auto s_r = m_session.request(s_req, 500);
    if (!s_r) throw CommandException("enable failed: ack timeout");
}

void ArmImpl::drag() {
    fci::arm::SetArmModeRequestPacket s_req{};
    s_req.payload.mode = fci::arm::ArmMode::Drag;
    auto s_r = m_session.request(s_req, 500);
    if (!s_r) throw CommandException("drag failed: ack timeout");
}

void ArmImpl::disable() {
    fci::arm::SetArmModeRequestPacket s_req{};
    s_req.payload.mode = fci::arm::ArmMode::Damp;
    auto s_r = m_session.request(s_req, 500);
    if (!s_r) throw CommandException("disable failed: ack timeout");
}

void ArmImpl::stop() {
    m_stop_flag = true;
    m_running = false;
    m_data_ready.release();
}

} // namespace florid
