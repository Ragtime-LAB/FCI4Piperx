#ifndef FLORID_CORE_ARM_CORE_HPP
#define FLORID_CORE_ARM_CORE_HPP

#include "florid/ArmState.hpp"
#include "florid/ControlTypes.hpp"

#include "fci_protocol/arm/packets.hpp"
#include "fci_protocol/arm/constants.hpp"

#include <cstdint>
#include <cstring>

namespace florid {

class ArmCore {
public:
    ArmCore() = default;

    std::uint32_t nextSeq() { return ++m_seq_num; }

    // ── SDK type → protocol packet converters ──

    fci::arm::JointMITCommandPacket s_pack(const JointMIT& s_cmd);

private:
    std::uint32_t m_seq_num{0};
};

} // namespace florid

#endif // FLORID_CORE_ARM_CORE_HPP
