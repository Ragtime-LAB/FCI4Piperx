#include "florid/core/ArmCore.hpp"

namespace florid {

static void s_copyFloats(const float* s_src, float* s_dst, std::uint8_t s_n) {
    for (std::uint8_t s_i = 0; s_i < s_n; ++s_i) s_dst[s_i] = s_src[s_i];
}

fci::arm::JointMITCommandPacket ArmCore::s_pack(const JointMIT& s_cmd) {
    fci::arm::JointMITCommandPacket s_pkt{};
    s_copyFloats(s_cmd.m_q, s_pkt.q, 12);
    s_copyFloats(s_cmd.m_dq, s_pkt.dq, 12);
    s_copyFloats(s_cmd.m_tau, s_pkt.tau, 12);
    s_copyFloats(s_cmd.m_kp, s_pkt.kp, 12);
    s_copyFloats(s_cmd.m_kd, s_pkt.kd, 12);
    s_pkt.control_mode = 1; // MIT mode
    if (s_cmd.m_firmware_gravity) s_pkt.control_mode |= 0x04; // bind bit2 = gravity_enable
    s_pkt.seq = static_cast<std::uint16_t>(nextSeq());
    return s_pkt;
}

} // namespace florid
