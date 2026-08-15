#ifndef FCI_PROTOCOL_UPGRADE_TRANSPORT_HPP
#define FCI_PROTOCOL_UPGRADE_TRANSPORT_HPP

#include "fci_protocol/session/upgrade_control_session.hpp"

namespace fci::upgrade {

template <typename TickProvider>
using Transport = fci::session::UpgradeControlCallbackSession<TickProvider>;

} // namespace fci::upgrade

#endif // FCI_PROTOCOL_UPGRADE_TRANSPORT_HPP
