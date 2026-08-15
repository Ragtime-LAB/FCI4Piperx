#ifndef FLORID_CONTROL_TYPES_HPP
#define FLORID_CONTROL_TYPES_HPP

#include <cstdint>

namespace florid {

enum class ReconnectPolicy {
    kThrow,
    kWait,
};

enum class ControllerMode : std::uint8_t {
    JointImpedance = 0,
};

enum class ComputationMode : std::uint8_t {
    kHost = 0,
    kFirmware = 1,
};

struct Finishable {
    bool m_motion_finished{false};
};

struct JointMIT : Finishable {
    static constexpr std::size_t kCount = 12; // 6 per arm x 2 arms
    float m_q[kCount]{};
    float m_dq[kCount]{};
    float m_tau[kCount]{};
    float m_kp[kCount]{};
    float m_kd[kCount]{};
    bool m_firmware_gravity{false};

    static JointMIT MotionFinished(const JointMIT& s_cmd) {
        JointMIT s_r = s_cmd;
        s_r.m_motion_finished = true;
        return s_r;
    }
};

struct TorqueControlDiagnostics {
    double m_actual_hz{};
    std::uint64_t m_period_us_avg{};
    std::uint64_t m_period_us_max{};
    std::uint64_t m_overrun_count{};
    std::uint64_t m_command_age_us{};
    std::uint64_t m_sent_count{};
    std::uint64_t m_last_sdk_timestamp_us{};
    std::uint32_t m_last_sdk_seq{};
    std::uint64_t m_state_age_us{};
    std::uint64_t m_stale_command_count{};
};

} // namespace florid

#endif // FLORID_CONTROL_TYPES_HPP
