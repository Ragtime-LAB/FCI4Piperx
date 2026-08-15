#ifndef FLORID_ARM_STATE_HPP
#define FLORID_ARM_STATE_HPP

#include <cstdint>

namespace florid {

struct ArmState {
    double m_time{};
    std::uint32_t m_seq{};
    std::uint32_t m_mode{};
    std::uint64_t m_source_timestamp_us{};
    std::uint32_t m_errors{};
    float m_q[12]{};
    float m_dq[12]{};
    float m_tau[12]{};
    float m_base_gravity[6]{};   // per arm [arm][xyz]
    float m_O_T_EE[32]{};        // per arm column-major 4x4
    float m_F_ext[12]{};         // per arm [arm][wrench]
    float m_gripper_q[2]{};
    float m_gripper_dq[2]{};
    float m_gripper_tau[2]{};
};

} // namespace florid

#endif // FLORID_ARM_STATE_HPP
