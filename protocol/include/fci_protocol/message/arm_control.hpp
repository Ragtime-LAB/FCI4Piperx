#ifndef FCI_PROTOCOL_MESSAGE_ARM_CONTROL_HPP
#define FCI_PROTOCOL_MESSAGE_ARM_CONTROL_HPP

#include <array>
#include <cstdint>
#include <type_traits>

namespace fci::message::arm_control {

enum class ModeRequestType : std::uint8_t {
    None = 0,
    Damp,
    Drag,
    Pc,
};

enum class ModeRequestPriority : std::uint8_t {
    Low = 0,
    Normal,
    High,
    Critical,
};

enum class PcControlKind : std::uint8_t {
    None = 0,
    MIT,
    MitGravity,
};

enum class SessionEventType : std::uint8_t {
    None = 0,
    TransportConnected,
    TransportDisconnected,
    SessionStart,
    SessionStop,
    HeartbeatTimeout,
};

enum class CommandKind : std::uint8_t {
    None = 0,
    ModeRequest,
    JointCommand,
    GripperCommand,
    SessionEvent,
    ControlMode,
};

struct ModeCommand {
    ModeRequestType type{ModeRequestType::None};
    ModeRequestPriority priority{ModeRequestPriority::Low};
    std::uint8_t flags{0};
};

struct SessionCommand {
    SessionEventType type{SessionEventType::None};
    std::uint8_t flags{0};
};

struct ControlModeCommand {
    std::uint8_t mode{0};
    bool gripper{false};
};

struct JointCommand {
    PcControlKind control_kind{PcControlKind::None};
    std::uint8_t flags{0};
    std::array<float, 12> q{};
    std::array<float, 12> dq{};
    std::array<float, 12> tau{};
    std::array<float, 12> kp{};
    std::array<float, 12> kd{};
    std::array<float, 12> aux{};
};

struct GripperCommand {
    PcControlKind control_kind{PcControlKind::None};
    std::array<float, 2> q{};
    std::array<float, 2> dq{};
    std::array<float, 2> tau{};
    std::array<float, 2> kp{};
    std::array<float, 2> kd{};
    std::array<float, 2> aux{};
    std::uint8_t flags{0};
};

struct ArmCommandMsg {
    CommandKind kind{CommandKind::None};
    std::uint8_t reserved[3]{};
    ModeCommand mode{};
    SessionCommand session{};
    ControlModeCommand control_mode{};
    JointCommand joint{};
    GripperCommand gripper{};
};

static_assert(std::is_trivially_copyable_v<ArmCommandMsg>,
              "ArmCommandMsg must stay trivially copyable");

} // namespace fci::message::arm_control

#endif // FCI_PROTOCOL_MESSAGE_ARM_CONTROL_HPP
