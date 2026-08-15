#include "florid/Gripper.hpp"
#include "florid/Arm.hpp"
#include "florid/detail/ArmImpl.hpp"

namespace florid {

Gripper::Gripper(Arm& s_arm) {
    m_impl = s_arm.m_impl;
}

GripperState Gripper::readOnce() {
    if (!m_impl) return GripperState{};
    auto s_state = m_impl->readOnce();
    GripperState s_out{};
    for (int s_i = 0; s_i < 2; ++s_i) {
        s_out.m_q[s_i]   = s_state.m_gripper_q[s_i];
        s_out.m_dq[s_i]  = s_state.m_gripper_dq[s_i];
        s_out.m_tau[s_i] = s_state.m_gripper_tau[s_i];
    }
    return s_out;
}

void Gripper::control(std::function<JointMIT(const ArmState&, ArmControl&)> s_cb) {
    auto s_core = &m_core;
    m_impl->s_gripperLoop(s_cb, [s_core, this](const JointMIT& s_cmd) {
        auto s_pkt = s_core->s_pack(s_cmd);
        s_pkt.sdk_timestamp_us = detail::s_nowUs();
        m_impl->s_notify(s_pkt);
    });
}

std::unique_ptr<ActiveControl<JointMIT>> Gripper::startJointMITControl() {
    auto s_impl = m_impl;
    auto s_core = std::make_shared<GripperCore>(m_core);
    return std::make_unique<ActiveControl<JointMIT>>(
        [s_impl] { return s_impl->readOnce(); },
        [s_impl, s_core](const JointMIT& s_cmd) {
            auto s_pkt = s_core->s_pack(s_cmd);
            s_pkt.sdk_timestamp_us = detail::s_nowUs();
            s_impl->s_notify(s_pkt);
        });
}

} // namespace florid
