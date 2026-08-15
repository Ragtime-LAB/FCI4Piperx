#ifndef FLORID_ARM_HPP
#define FLORID_ARM_HPP

#include "florid/ArmControl.hpp"
#include "florid/ArmState.hpp"
#include "florid/ControlTypes.hpp"
#include "florid/Duration.hpp"
#include "florid/Errors.hpp"
#include "florid/core/ActiveControl.hpp"
#include "florid/Gripper.hpp"
#include "florid/recording/Timeline.hpp"

#include "fci_protocol/arm/constants.hpp"
#include "fci_protocol/arm/device_info.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace florid {

class ArmImpl;

class Arm {
public:
    static constexpr std::size_t s_kNumJoints = 12;

    static std::unique_ptr<Arm> create(const std::string& s_uri);

    Arm(Arm&& s_other) noexcept;
    Arm& operator=(Arm&& s_other) noexcept;
    ~Arm();

    Arm(const Arm&) = delete;
    Arm& operator=(const Arm&) = delete;

    // ── Control loop (blocking, runs callback in internal thread) ──

    void control(std::function<JointMIT(const ArmState&, ArmControl&)> s_cb);

    // ── State reading ──

    ArmState readOnce();
    std::optional<recording::TriggerEvent> readTriggerOnce();
    std::optional<recording::InterpolatedState> interpolateAt(
        std::uint64_t timestamp_mcu_us) const;

    template <typename Callable>
    void read(Callable&& s_cb) {
        while (true) {
            auto s_state = readOnce();
            if (s_state.m_seq == 0) continue;
            if (!s_cb(s_state)) break;
        }
    }

    Gripper& gripper();

    // ── Active control (manual read/write loop, pybind-friendly) ──

    std::unique_ptr<ActiveControl<JointMIT>> startJointMITControl();

    // ── Configuration ──

    void enable();
    void drag();
    void disable();
    void stop();

    std::uint32_t firmwarePeriodUs() const;
    ReconnectPolicy reconnectPolicy() const;
    void setReconnectPolicy(ReconnectPolicy s_p);
    bool isConnected() const;
    const fci::arm::DeviceInfo& deviceInfo() const;

private:
    Arm() = default;
    std::shared_ptr<ArmImpl> m_impl;
    friend class Gripper;
};

} // namespace florid

#endif // FLORID_ARM_HPP
