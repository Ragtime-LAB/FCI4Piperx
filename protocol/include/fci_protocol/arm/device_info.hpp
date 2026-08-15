#ifndef FCI_PROTOCOL_ARM_DEVICE_INFO_HPP
#define FCI_PROTOCOL_ARM_DEVICE_INFO_HPP

#include <cstdint>
#include <array>

#include "fci_protocol/version.hpp"
#include "fci_protocol/arm/constants.hpp"

#pragma pack(push, 1)

namespace fci::arm {

// ──────────────────────────────────────────────
//  Device info (firmware → host)
// ──────────────────────────────────────────────

struct DeviceInfo {
    fci::Semver protocol_version;  // kProtocolVersion as semver
    fci::Semver fw_version;        // firmware semantic version
    std::array<char, 32> board_name;
    std::array<char, 32> custom_name;
    std::uint8_t fw_type;          // FirmwareType enum
};

// ──────────────────────────────────────────────
//  Request / response payloads
// ──────────────────────────────────────────────

struct GetDeviceInfoCommand {
    std::uint8_t dummy;
};

struct GetDeviceInfoResponse {
    DeviceInfo info;
};

// ──────────────────────────────────────────────
//  Request / response packets (host → firmware)
// ──────────────────────────────────────────────

struct GetDeviceInfoRequestPacket {
    ReqId req_id;
    GetDeviceInfoCommand payload;
};

struct GetDeviceInfoResponsePacket {
    ReqId req_id;
    GetDeviceInfoResponse payload;
};

struct SetDeviceInfoCommand {
    std::array<char, 32> custom_name;
    std::uint8_t fw_type;           // 0xFF means "keep current value"
};

enum class SetDeviceInfoStatus : std::uint8_t {
    Ok = 0,
    StorageFailed = 1,
    NameTooLong = 2,
};

struct SetDeviceInfoRequestPacket {
    ReqId req_id;
    SetDeviceInfoCommand payload;
};

} // namespace fci::arm

#pragma pack(pop)

#endif // FCI_PROTOCOL_ARM_DEVICE_INFO_HPP
