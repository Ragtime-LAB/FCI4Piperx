#ifndef FCI_PROTOCOL_SESSION_STREAM_SESSION_HPP
#define FCI_PROTOCOL_SESSION_STREAM_SESSION_HPP

#include "RPL/Packets/USBAck.hpp"
#include "RPL/USBTransport.hpp"

namespace fci::session {

template <typename TickProvider, typename SendTransport, typename... Packets>
using StreamSession = RPL::USBTransport<
    RPL::AckManager<TickProvider>,
    SendTransport,
    USBAck,
    Packets...>;

} // namespace fci::session

#endif // FCI_PROTOCOL_SESSION_STREAM_SESSION_HPP
