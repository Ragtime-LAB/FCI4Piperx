#ifndef FCI_PROTOCOL_UPGRADE_PACKETS_HPP
#define FCI_PROTOCOL_UPGRADE_PACKETS_HPP

#include "fci_protocol/upgrade/constants.hpp"
#include "RPL/Meta/PacketTraits.hpp"
#include "RPL/Packets/USBAck.hpp"

#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)

namespace fci::upgrade {

struct Version {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t revision;
    std::uint8_t build;
};

struct GetBootStatusRequest {
    std::uint8_t req_id;
};

struct BootStatusResponse {
    std::uint8_t req_id;
    std::uint8_t protocol_version;
    std::uint8_t boot_state;
    std::uint8_t recovery_reason;
    std::uint8_t active_slot;
    std::uint8_t confirmed_slot;
    std::uint8_t pending_slot;
    std::uint8_t reserved0;
    std::uint16_t write_align;
    std::uint16_t max_chunk_size;
    std::uint32_t slot0_address;
    std::uint32_t slot0_size;
    std::uint32_t slot1_address;
    std::uint32_t slot1_size;
    std::uint32_t bytes_written;
};

struct StartUpgradeRequest {
    std::uint8_t req_id;
    std::uint8_t image_id;
    std::uint8_t target_slot;
    std::uint8_t flags;
    Version version;
};

struct StartUpgradeResponse {
    std::uint8_t req_id;
    std::uint8_t image_id;
    std::uint8_t target_slot;
    std::uint8_t accepted_flags;
    std::uint32_t session_id;
    std::uint32_t next_offset;
    std::uint16_t write_align;
    std::uint16_t max_chunk_size;
};

struct RebootRequest {
    std::uint8_t req_id;
    std::uint8_t mode;
    std::uint8_t reserved0;
    std::uint8_t reserved1;
};

struct RebootingResponse {
    std::uint8_t req_id;
    std::uint8_t mode;
    std::uint8_t reserved0;
    std::uint8_t reserved1;
};

static_assert(sizeof(Version) == 4);
static_assert(sizeof(GetBootStatusRequest) <= 0xFF);
static_assert(sizeof(BootStatusResponse) <= 0xFF);
static_assert(sizeof(StartUpgradeRequest) <= 0xFF);
static_assert(sizeof(StartUpgradeResponse) <= 0xFF);
static_assert(sizeof(RebootRequest) <= 0xFF);
static_assert(sizeof(RebootingResponse) <= 0xFF);

} // namespace fci::upgrade

#pragma pack(pop)

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::GetBootStatusRequest>
    : PacketTraitsBase<PacketTraits<fci::upgrade::GetBootStatusRequest>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::GetBootStatusRequest);
    static constexpr std::size_t size = sizeof(fci::upgrade::GetBootStatusRequest);
    static constexpr PacketCategory category = PacketCategory::Request;
    using Protocol = USBRequestProto;
};

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::BootStatusResponse>
    : PacketTraitsBase<PacketTraits<fci::upgrade::BootStatusResponse>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::BootStatusResponse);
    static constexpr std::size_t size = sizeof(fci::upgrade::BootStatusResponse);
    using Protocol = USBBaseProto;
};

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::StartUpgradeRequest>
    : PacketTraitsBase<PacketTraits<fci::upgrade::StartUpgradeRequest>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::StartUpgradeRequest);
    static constexpr std::size_t size = sizeof(fci::upgrade::StartUpgradeRequest);
    static constexpr PacketCategory category = PacketCategory::Request;
    using Protocol = USBRequestProto;
};

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::StartUpgradeResponse>
    : PacketTraitsBase<PacketTraits<fci::upgrade::StartUpgradeResponse>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::StartUpgradeResponse);
    static constexpr std::size_t size = sizeof(fci::upgrade::StartUpgradeResponse);
    using Protocol = USBBaseProto;
};

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::RebootRequest>
    : PacketTraitsBase<PacketTraits<fci::upgrade::RebootRequest>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::RebootRequest);
    static constexpr std::size_t size = sizeof(fci::upgrade::RebootRequest);
    static constexpr PacketCategory category = PacketCategory::Request;
    using Protocol = USBRequestProto;
};

template <>
struct RPL::Meta::PacketTraits<fci::upgrade::RebootingResponse>
    : PacketTraitsBase<PacketTraits<fci::upgrade::RebootingResponse>> {
    static constexpr std::uint16_t cmd = fci::upgrade::to_u8(
        fci::upgrade::Command::RebootingResponse);
    static constexpr std::size_t size = sizeof(fci::upgrade::RebootingResponse);
    using Protocol = USBBaseProto;
};

#endif // FCI_PROTOCOL_UPGRADE_PACKETS_HPP
