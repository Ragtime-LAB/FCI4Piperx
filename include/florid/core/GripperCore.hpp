#ifndef FLORID_CORE_GRIPPER_CORE_HPP
#define FLORID_CORE_GRIPPER_CORE_HPP

#include "florid/ControlTypes.hpp"

#include "fci_protocol/arm/packets.hpp"

#include <cstdint>

namespace florid {

class GripperCore {
public:
    std::uint32_t nextSeq() { return ++m_seq_num; }

    fci::arm::GripperCommandPacket s_pack(const JointMIT& s_cmd);

private:
    std::uint32_t m_seq_num{0};
};

} // namespace florid

#endif // FLORID_CORE_GRIPPER_CORE_HPP
