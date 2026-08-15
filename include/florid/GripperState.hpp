#ifndef FLORID_GRIPPER_STATE_HPP
#define FLORID_GRIPPER_STATE_HPP

#include <cstdint>

namespace florid {

struct GripperState {
    float m_q[2]{};
    float m_dq[2]{};
    float m_tau[2]{};
};

} // namespace florid

#endif // FLORID_GRIPPER_STATE_HPP
