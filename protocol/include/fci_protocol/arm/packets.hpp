#ifndef FCI_PROTOCOL_ARM_PACKETS_HPP
#define FCI_PROTOCOL_ARM_PACKETS_HPP

#include "fci_protocol/arm/constants.hpp"
#include "fci_protocol/arm/device_info.hpp"
#include "RPL/Meta/PacketTraits.hpp"
#include "RPL/Packets/USBAck.hpp"

#include <cstddef>
#include <cstdint>
#include <array>

#pragma pack(push, 1)

namespace fci::arm {

// Joint indexing convention: 0..5 = ARM1, 6..11 = ARM2.
// Gripper indexing convention: 0 = ARM1, 1 = ARM2.
inline constexpr std::size_t kArmJointCount = 6;
inline constexpr std::size_t kDualJointCount = 12;
inline constexpr std::size_t kGripperCount = 2;

// ──────────────────────────────────────────────
//  Telemetry (firmware → host)
// ──────────────────────────────────────────────

struct JointStatus {
    std::array<float, kDualJointCount> q;
    std::array<float, kDualJointCount> dq;
    std::array<float, kDualJointCount> tau;
};

struct GripperStatus {
    std::array<float, kGripperCount> q;
    std::array<float, kGripperCount> dq;
    std::array<float, kGripperCount> tau;
};

struct ArmStatus {
    ArmMode mode;
    std::uint32_t seq;
    std::uint64_t timestamp_us;
    JointStatus status;
    float base_gravity[2 * 3];      // per arm, column-major [arm][xyz]
    GripperStatus gripper;
    float O_T_EE[2 * 16];           // per arm end-effector pose (column-major 4x4)
    float F_ext[2 * 6];             // per arm estimated external wrench
    std::uint32_t errors;           // error bitmap
    std::uint64_t last_sdk_timestamp_us; // echo: most recent control packet's sdk timestamp
};

// ──────────────────────────────────────────────
//  Ack
// ──────────────────────────────────────────────

struct AckPacket {
    ReqId req_id;
    std::uint16_t cmd_id;
    std::uint8_t status;
};

// ──────────────────────────────────────────────
//  Command payloads (request body)
// ──────────────────────────────────────────────

struct ArmControlModeCommand {
    MotorControlMode mode;
};

struct GripperControlModeCommand {
    MotorControlMode mode;
};

struct SetArmModeCommand {
    ArmMode mode;
};

struct ClearFaultsCommand {
    std::uint8_t dummy;
};

struct SdkClientConnectedCommand {
    std::uint8_t dummy;
};

struct SdkClientDisconnectedCommand {
    std::uint8_t dummy;
};

// ──────────────────────────────────────────────
//  Request packets (host → firmware)
// ──────────────────────────────────────────────

struct ArmControlModeRequestPacket {
    ReqId req_id;
    ArmControlModeCommand payload;
};

struct GripperControlModeRequestPacket {
    ReqId req_id;
    GripperControlModeCommand payload;
};

struct SetArmModeRequestPacket {
    ReqId req_id;
    SetArmModeCommand payload;
};

struct ClearFaultsRequestPacket {
    ReqId req_id;
    ClearFaultsCommand payload;
};

struct SdkClientConnectedRequestPacket {
    ReqId req_id;
    SdkClientConnectedCommand payload;
};

struct SdkClientDisconnectedRequestPacket {
    ReqId req_id;
    SdkClientDisconnectedCommand payload;
};

// ──────────────────────────────────────────────
//  Real-time control (fire-and-forget, no tx_id)
// ──────────────────────────────────────────────

struct JointMITCommandPacket {
    float q[kDualJointCount];
    float dq[kDualJointCount];
    float tau[kDualJointCount];
    float kp[kDualJointCount];
    float kd[kDualJointCount];
    std::uint32_t dt_us;
    std::uint16_t seq;
    std::uint8_t control_mode;
    std::uint64_t sdk_timestamp_us; // host monotonic timestamp when packet was sent
};

struct GripperCommandPacket {
    float q[kGripperCount];
    float dq[kGripperCount];
    float tau[kGripperCount];
    float kp[kGripperCount];
    float kd[kGripperCount];
    std::uint32_t dt_us;
    std::uint16_t seq;
    std::uint8_t control_mode;
    std::uint64_t sdk_timestamp_us;
};

} // namespace fci::arm

// ══════════════════════════════════════════════
//  RPL PacketTraits specialisations
// ══════════════════════════════════════════════

namespace RPL::Meta {

template <>
struct PacketTraits<fci::arm::ArmStatus>
    : PacketTraitsBase<PacketTraits<fci::arm::ArmStatus>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::ArmStatus);
    static constexpr std::size_t size = sizeof(fci::arm::ArmStatus);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Notification;
};

template <>
struct PacketTraits<fci::arm::GripperStatus>
    : PacketTraitsBase<PacketTraits<fci::arm::GripperStatus>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::GripperStatus);
    static constexpr std::size_t size = sizeof(fci::arm::GripperStatus);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Notification;
};

template <>
struct PacketTraits<fci::arm::AckPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::AckPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::Ack);
    static constexpr std::size_t size = sizeof(fci::arm::AckPacket);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Ack;
};

template <>
struct PacketTraits<fci::arm::ArmControlModeRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::ArmControlModeRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::ArmControlModeRequest);
    static constexpr std::size_t size = sizeof(fci::arm::ArmControlModeRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::GripperControlModeRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::GripperControlModeRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::GripperControlModeRequest);
    static constexpr std::size_t size = sizeof(fci::arm::GripperControlModeRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::SetArmModeRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::SetArmModeRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::SetArmModeRequest);
    static constexpr std::size_t size = sizeof(fci::arm::SetArmModeRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::ClearFaultsRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::ClearFaultsRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::ClearFaultsRequest);
    static constexpr std::size_t size = sizeof(fci::arm::ClearFaultsRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::SdkClientConnectedRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::SdkClientConnectedRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::SdkClientConnectedRequest);
    static constexpr std::size_t size = sizeof(fci::arm::SdkClientConnectedRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::SdkClientDisconnectedRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::SdkClientDisconnectedRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::SdkClientDisconnectedRequest);
    static constexpr std::size_t size = sizeof(fci::arm::SdkClientDisconnectedRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::GetDeviceInfoRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::GetDeviceInfoRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::GetDeviceInfoRequest);
    static constexpr std::size_t size = sizeof(fci::arm::GetDeviceInfoRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::GetDeviceInfoResponsePacket>
    : PacketTraitsBase<PacketTraits<fci::arm::GetDeviceInfoResponsePacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::GetDeviceInfoResponse);
    static constexpr std::size_t size = sizeof(fci::arm::GetDeviceInfoResponsePacket);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Notification;
};

template <>
struct PacketTraits<fci::arm::SetDeviceInfoRequestPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::SetDeviceInfoRequestPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::SetDeviceInfoRequest);
    static constexpr std::size_t size = sizeof(fci::arm::SetDeviceInfoRequestPacket);
    using Protocol = USBRequestProto;
    static constexpr PacketCategory category = PacketCategory::Request;
};

template <>
struct PacketTraits<fci::arm::JointMITCommandPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::JointMITCommandPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::JointMITCommand);
    static constexpr std::size_t size = sizeof(fci::arm::JointMITCommandPacket);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Notification;
};

template <>
struct PacketTraits<fci::arm::GripperCommandPacket>
    : PacketTraitsBase<PacketTraits<fci::arm::GripperCommandPacket>> {
    static constexpr std::uint16_t cmd = fci::arm::to_u16(fci::arm::Command::GripperCommand);
    static constexpr std::size_t size = sizeof(fci::arm::GripperCommandPacket);
    using Protocol = USBBaseProto;
    static constexpr PacketCategory category = PacketCategory::Notification;
};

} // namespace RPL::Meta

#pragma pack(pop)

#endif // FCI_PROTOCOL_ARM_PACKETS_HPP
