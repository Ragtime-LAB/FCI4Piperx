#ifndef FCI_PROTOCOL_SESSION_ARM_CONTROL_SESSION_HPP
#define FCI_PROTOCOL_SESSION_ARM_CONTROL_SESSION_HPP

#include "fci_protocol/message/arm.hpp"
#include "fci_protocol/session/stream_session.hpp"
#include "fci_protocol/transport/byte_stream_transport.hpp"

namespace fci::session {

template <typename TickProvider, typename SendTransport>
using ArmControlSession = StreamSession<
    TickProvider,
    SendTransport,
    fci::arm::ArmStatus,
    fci::arm::GripperStatus,
    fci::arm::TriggerPacket,
    fci::arm::AckPacket,
    fci::arm::ArmControlModeRequestPacket,
    fci::arm::GripperControlModeRequestPacket,
    fci::arm::SetArmModeRequestPacket,
    fci::arm::ClearFaultsRequestPacket,
    fci::arm::SdkClientConnectedRequestPacket,
    fci::arm::SdkClientDisconnectedRequestPacket,
    fci::arm::GetDeviceInfoRequestPacket,
    fci::arm::GetDeviceInfoResponsePacket,
    fci::arm::SetDeviceInfoRequestPacket,
    fci::arm::JointMITCommandPacket,
    fci::arm::GripperCommandPacket>;

template <typename TickProvider>
using ArmControlCallbackSession =
    ArmControlSession<TickProvider, fci::transport::FunctionPointerByteStreamTransport>;

} // namespace fci::session

#endif // FCI_PROTOCOL_SESSION_ARM_CONTROL_SESSION_HPP
