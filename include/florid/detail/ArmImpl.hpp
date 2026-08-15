#ifndef FLORID_DETAIL_ARM_IMPL_HPP
#define FLORID_DETAIL_ARM_IMPL_HPP

#include "florid/ArmControl.hpp"
#include "florid/ArmState.hpp"
#include "florid/ControlTypes.hpp"
#include "florid/Duration.hpp"
#include "florid/core/ArmCore.hpp"
#include "florid/core/traits.hpp"
#include "florid/detail/Transport.hpp"
#include "florid/detail/TickProvider.hpp"
#include "florid/detail/LatencyEstimator.hpp"
#include "florid/recording/Timeline.hpp"

#include "fci_protocol/session/arm_control_session.hpp"
#include "fci_protocol/transport/byte_stream_transport.hpp"
#include "fci_protocol/arm/packets.hpp"
#include "fci_protocol/arm/device_info.hpp"
#include "fci_protocol/arm/constants.hpp"

#include "readerwriterqueue.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <semaphore>
#include <chrono>
#include <mutex>
#include <thread>
#include <span>
namespace florid {

class ArmImpl {

public:
    using SendFunc = std::function<void(const std::uint8_t*, std::size_t)>;
    using Session = fci::session::ArmControlSession<detail::MonotonicTickProvider, SendFunc>;

    explicit ArmImpl(std::unique_ptr<Transport> s_transport);
    ~ArmImpl();

    ArmImpl(const ArmImpl&) = delete;
    ArmImpl& operator=(const ArmImpl&) = delete;

    // ── Receive pipeline ──
    static void s_onPhysData(void* s_context, const std::uint8_t* s_data, std::size_t s_size);

    // ── Device info ──
    const fci::arm::DeviceInfo& getDeviceInfo() const { return m_device_info; }
    std::uint32_t firmwarePeriodUs() const { return m_fw_dt_us; }

    // ── Arm state ──
    ArmState readOnce();
    std::optional<recording::TriggerEvent> readTriggerOnce();
    std::optional<recording::InterpolatedState> interpolateAt(
        std::uint64_t s_timestamp_mcu_us) const;
    ArmControl& controlHandle() { return m_arm_control; }

    // ── Control loop (template, called from Arm) ──
    template <typename Callback>
    void s_controlLoop(Callback s_cb) {
        using ReturnType = std::decay_t<decltype(s_cb(std::declval<const ArmState&>(),
                                                      std::declval<ArmControl&>()))>;

        m_running = true;
        m_stop_flag = false;

        s_requestPcMode();

        while (m_running && !m_stop_flag) {
            m_data_ready.acquire();

            fci::arm::ArmStatus s_raw;
            if (!m_rx_queue.try_dequeue(s_raw)) continue;

            ArmState s_state = s_convertStatus(s_raw);

            auto s_cmd = s_cb(s_state, m_arm_control);
            s_sendCommand(s_cmd);

            if (s_cmd.m_motion_finished) break;
        }

        m_running = false;
    }

    // ── Gripper control loop (packs externally) ──
    template <typename Callback, typename Packer>
    void s_gripperLoop(Callback s_cb, Packer s_packer) {
        using ReturnType = std::decay_t<decltype(s_cb(std::declval<const ArmState&>(),
                                                      std::declval<ArmControl&>()))>;

        m_running = true;
        m_stop_flag = false;

        s_requestPcMode();

        while (m_running && !m_stop_flag) {
            m_data_ready.acquire();

            fci::arm::ArmStatus s_raw;
            if (!m_rx_queue.try_dequeue(s_raw)) continue;

            ArmState s_state = s_convertStatus(s_raw);

            auto s_cmd = s_cb(s_state, m_arm_control);
            s_packer(s_cmd);

            if (s_cmd.m_motion_finished) break;
        }

        m_running = false;
    }

    // ── Configuration ──
    void enable();
    void drag();
    void disable();
    void stop();

    // ── Connection ──
    bool isConnected() const { return m_connected.load(); }
    ReconnectPolicy reconnectPolicy() const { return m_reconnect_policy; }
    void setReconnectPolicy(ReconnectPolicy s_p) { m_reconnect_policy = s_p; }

    // ── Prepare control mode (used by Arm::start*Control) ──
    template <typename CommandType>
    void s_prepareControl() {
        s_requestPcMode();
    }

    // ── Send (public, used by ActiveControl lambdas) ──
    template <typename CommandType>
    void s_sendCommand(const CommandType& s_cmd) {
        auto s_pkt = m_arm_core.s_pack(s_cmd);
        s_pkt.sdk_timestamp_us = detail::s_nowUs();
        m_session.notify(s_pkt);
    }

    // ── Generic notify (used by Gripper to send through shared session) ──
    template <typename ProtoPacket>
    void s_notify(const ProtoPacket& s_pkt) {
        m_session.notify(s_pkt);
    }

    ArmCore m_arm_core;
    Session m_session;

private:
    void s_feedBytes(const std::uint8_t* s_data, std::size_t s_size);
    void s_onPacket(std::uint16_t s_cmd,
                    std::span<const std::uint8_t> s_payload1,
                    std::span<const std::uint8_t> s_payload2);
    void s_fetchDeviceInfo();
    void s_requestPcMode();

    ArmState s_convertStatus(const fci::arm::ArmStatus& s_raw);

    // ── Physical transport ──
    std::unique_ptr<Transport> m_transport;

    // ── SPSC queue ──
    moodycamel::ReaderWriterQueue<fci::arm::ArmStatus> m_rx_queue{64};
    moodycamel::ReaderWriterQueue<recording::TriggerEvent> m_trigger_queue{128};
    std::counting_semaphore<65536> m_data_ready{0};
    std::uint32_t m_last_status_seq{0};

    // ── Cached DeviceInfo ──
    fci::arm::DeviceInfo m_device_info{};
    std::uint32_t m_fw_dt_us{10000}; // 100 Hz

    // ── Connection ──
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_running{false};
    ReconnectPolicy m_reconnect_policy{ReconnectPolicy::kThrow};
    std::chrono::milliseconds m_recv_timeout{50};

    // ── Control ──
    ArmControl m_arm_control;
    std::mutex m_control_mutex;
    std::atomic<bool> m_reconnecting{false};
    std::atomic<bool> m_stop_flag{false};
    detail::LatencyEstimator m_latency;
    recording::AngleTimeline m_timeline;

    friend class ArmControl;
};

} // namespace florid

#endif // FLORID_DETAIL_ARM_IMPL_HPP
