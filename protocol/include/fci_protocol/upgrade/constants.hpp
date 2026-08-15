#ifndef FCI_PROTOCOL_UPGRADE_CONSTANTS_HPP
#define FCI_PROTOCOL_UPGRADE_CONSTANTS_HPP

#include <cstdint>

#include "fci_protocol/version.hpp"

namespace fci::upgrade {

inline constexpr fci::Semver kProtocolVersion = fci::MakeSemver(1, 0, 0);

enum class Command : std::uint8_t {
    GetBootStatusRequest = 0x10,
    BootStatusResponse = 0x11,
    StartUpgradeRequest = 0x12,
    StartUpgradeResponse = 0x13,
    RebootRequest = 0x1C,
    RebootingResponse = 0x1D,
};

enum class AckStatus : std::uint8_t {
    Ok = 0x00,
    Busy = 0x01,
    InvalidState = 0x02,
    InvalidArgument = 0x03,
    OutOfRange = 0x04,
    CrcMismatch = 0x05,
    FlashFailure = 0x06,
    VerifyFailure = 0x07,
    SignatureFailure = 0x08,
    Unsupported = 0x09,
    InternalError = 0x0A,
};

enum class ImageId : std::uint8_t {
    Application = 0,
};

enum class SlotId : std::uint8_t {
    None = 0,
    Primary = 1,
    Secondary = 2,
};

enum class BootState : std::uint8_t {
    Idle = 0,
    Recovery = 1,
    Upgrading = 2,
    PendingTest = 3,
    Booting = 4,
};

enum class RecoveryReason : std::uint8_t {
    None = 0,
    KeyPressed = 1,
    NoBootableImage = 2,
    ImageValidationFailed = 3,
    UpgradeRequested = 4,
    InternalFailure = 5,
};

enum class StartUpgradeFlags : std::uint8_t {
    None = 0,
    Resume = 1u << 0,
    ForceErase = 1u << 1,
};

enum class RebootMode : std::uint8_t {
    Normal = 0,
    StayInRecovery = 1,
    TryBootImmediately = 2,
};

constexpr std::uint8_t to_u8(Command value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(AckStatus value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(ImageId value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(SlotId value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(BootState value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(RecoveryReason value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(StartUpgradeFlags value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr std::uint8_t to_u8(RebootMode value)
{
    return static_cast<std::uint8_t>(value);
}

} // namespace fci::upgrade

#endif // FCI_PROTOCOL_UPGRADE_CONSTANTS_HPP
