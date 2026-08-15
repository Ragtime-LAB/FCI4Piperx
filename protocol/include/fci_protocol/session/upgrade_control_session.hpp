#ifndef FCI_PROTOCOL_SESSION_UPGRADE_CONTROL_SESSION_HPP
#define FCI_PROTOCOL_SESSION_UPGRADE_CONTROL_SESSION_HPP

#include "fci_protocol/message/upgrade.hpp"
#include "fci_protocol/session/stream_session.hpp"
#include "fci_protocol/transport/byte_stream_transport.hpp"

namespace fci::session {

template <typename TickProvider, typename SendTransport>
using UpgradeControlSession = StreamSession<
    TickProvider,
    SendTransport,
    fci::upgrade::GetBootStatusRequest,
    fci::upgrade::BootStatusResponse,
    fci::upgrade::StartUpgradeRequest,
    fci::upgrade::StartUpgradeResponse,
    fci::upgrade::RebootRequest,
    fci::upgrade::RebootingResponse>;

template <typename TickProvider>
using UpgradeControlCallbackSession =
    UpgradeControlSession<TickProvider, fci::transport::FunctionPointerByteStreamTransport>;

} // namespace fci::session

#endif // FCI_PROTOCOL_SESSION_UPGRADE_CONTROL_SESSION_HPP
