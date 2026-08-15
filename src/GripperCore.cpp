#include "florid/core/GripperCore.hpp"

namespace florid {

fci::arm::GripperCommandPacket GripperCore::s_pack(const JointMIT& s_cmd) {
    fci::arm::GripperCommandPacket s_pkt{};
    // JointMIT m_q[0/1] map to ARM1/ARM2 gripper.
    s_pkt.q[0]   = s_cmd.m_q[0];
    s_pkt.dq[0]  = s_cmd.m_dq[0];
    s_pkt.tau[0] = s_cmd.m_tau[0];
    s_pkt.kp[0]  = s_cmd.m_kp[0];
    s_pkt.kd[0]  = s_cmd.m_kd[0];
    s_pkt.q[1]   = s_cmd.m_q[1];
    s_pkt.dq[1]  = s_cmd.m_dq[1];
    s_pkt.tau[1] = s_cmd.m_tau[1];
    s_pkt.kp[1]  = s_cmd.m_kp[1];
    s_pkt.kd[1]  = s_cmd.m_kd[1];
    s_pkt.control_mode = 1;
    if (s_cmd.m_firmware_gravity) s_pkt.control_mode |= 0x04;
    s_pkt.seq = static_cast<std::uint16_t>(nextSeq());
    return s_pkt;
}

} // namespace florid
