#ifndef FCI_PROTOCOL_ARM_HPP
#define FCI_PROTOCOL_ARM_HPP

#include "fci_protocol/message/arm.hpp"
#include "fci_protocol/session/arm_control_session.hpp"
#include "fci_protocol/arm/constants.hpp"
#include "fci_protocol/arm/packets.hpp"

namespace fci::arm {

template <typename TickProvider>
using Transport = fci::session::ArmControlCallbackSession<TickProvider>;

} // namespace fci::arm

#endif // FCI_PROTOCOL_ARM_HPP
