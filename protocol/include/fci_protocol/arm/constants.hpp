#ifndef FCI_PROTOCOL_ARM_CONSTANTS_HPP
#define FCI_PROTOCOL_ARM_CONSTANTS_HPP

#include <cstdint>

#include "fci_protocol/version.hpp"

namespace fci::arm
{
    using ReqId = std::uint8_t;

    inline constexpr fci::Semver kProtocolVersion = fci::MakeSemver(0, 0, 1);

    enum class Command : std::uint16_t
    {
        // ── Telemetry (firmware → host, notification) ──
        ArmStatus = 0x6001,
        GripperStatus = 0x6003,
        Trigger = 0x6004,

        // ── Ack ──
        Ack = 0x6FF0,

        // ── Reliable requests (response via Ack status) ──
        ClearFaultsRequest = 0x6205,
        SdkClientConnectedRequest = 0x6209,
        SdkClientDisconnectedRequest = 0x620B,
        GetDeviceInfoRequest = 0x6215,
        SetDeviceInfoRequest = 0x6217,
        ArmControlModeRequest = 0x6219,
        GripperControlModeRequest = 0x621B,
        SetArmModeRequest = 0x6225,

        // ── Data-carrying responses (not covered by Ack) ──
        GetDeviceInfoResponse = 0x6216,

        // ── Real-time control (fire-and-forget, notification) ──
        JointMITCommand = 0x6301,
        GripperCommand = 0x6305,
    };

    // Motor control mode. This arm only supports MIT impedance control.
    enum class MotorControlMode : std::uint8_t
    {
        MIT = 1,
    };

    enum class FirmwareType : std::uint8_t
    {
        StandardArm = 0,
        MobileArm = 1,
        CobotArm = 2,
    };

    enum class ArmMode : std::uint8_t
    {
        Pc = 0,
        Drag = 1,
        Damp = 2,
    };

    // Ack status — unified across all Request packets
    enum class AckStatus : std::uint8_t
    {
        Ok = 0,
        Failed = 1,
    };

    constexpr std::uint16_t to_u16(Command value)
    {
        return static_cast<std::uint16_t>(value);
    }

    constexpr std::uint8_t to_u8(MotorControlMode value)
    {
        return static_cast<std::uint8_t>(value);
    }

    constexpr std::uint8_t to_u8(FirmwareType value)
    {
        return static_cast<std::uint8_t>(value);
    }

    constexpr std::uint8_t to_u8(ArmMode value)
    {
        return static_cast<std::uint8_t>(value);
    }
} // namespace fci::arm

#endif // FCI_PROTOCOL_ARM_CONSTANTS_HPP
