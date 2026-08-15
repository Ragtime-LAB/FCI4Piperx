#include "florid/Arm.hpp"
#include "florid/detail/ArmImpl.hpp"
#include "florid/detail/AstrialUSBTransport.hpp"

#include <memory>
#include <thread>

namespace florid {

std::unique_ptr<Arm> Arm::create(const std::string& s_uri) {
    // "usb:///dev/ttyACM0" or "usb://COM3" or "tcp://host:port"
    std::unique_ptr<Transport> s_transport;

    if (s_uri.starts_with("usb://")) {
        std::string s_path = s_uri.substr(6); // strip "usb://"
        s_transport = std::make_unique<AstrialUSBTransport>(s_path);
    } else if (s_uri.starts_with("mock://")) {
        // Mock transport for testing — created externally via Arm(std::shared_ptr<ArmImpl>)
        return nullptr;
    } else {
        return nullptr;
    }

    auto s_impl = std::make_shared<ArmImpl>(std::move(s_transport));
    auto s_arm = std::unique_ptr<Arm>(new Arm());
    s_arm->m_impl = s_impl;
    return s_arm;
}

Arm::Arm(Arm&& s_other) noexcept : m_impl(std::move(s_other.m_impl)) {}

Arm& Arm::operator=(Arm&& s_other) noexcept {
    if (this != &s_other) m_impl = std::move(s_other.m_impl);
    return *this;
}

Arm::~Arm() = default;

Gripper& Arm::gripper() {
    static Gripper s_gripper(*this);
    return s_gripper;
}

// ── Control loops ──

void Arm::control(std::function<JointMIT(const ArmState&, ArmControl&)> s_cb) {
    m_impl->s_controlLoop(std::move(s_cb));
}

// ── State reading ──

ArmState Arm::readOnce() {
    return m_impl->readOnce();
}

std::optional<recording::TriggerEvent> Arm::readTriggerOnce() {
    return m_impl->readTriggerOnce();
}

std::optional<recording::InterpolatedState>
Arm::interpolateAt(std::uint64_t s_timestamp_mcu_us) const {
    return m_impl->interpolateAt(s_timestamp_mcu_us);
}

// ── Active control ──

std::unique_ptr<ActiveControl<JointMIT>> Arm::startJointMITControl() {
    m_impl->s_prepareControl<JointMIT>();
    auto s_impl = m_impl;
    return std::make_unique<ActiveControl<JointMIT>>(
        [s_impl] { return s_impl->readOnce(); },
        [s_impl](const JointMIT& s_cmd) { s_impl->s_sendCommand(s_cmd); });
}

// ── Configuration ──

void Arm::enable() { m_impl->enable(); }
void Arm::drag() { m_impl->drag(); }
void Arm::disable() { m_impl->disable(); }
void Arm::stop() { m_impl->stop(); }

std::uint32_t Arm::firmwarePeriodUs() const { return m_impl->firmwarePeriodUs(); }
ReconnectPolicy Arm::reconnectPolicy() const { return m_impl->reconnectPolicy(); }
void Arm::setReconnectPolicy(ReconnectPolicy s_p) { m_impl->setReconnectPolicy(s_p); }
bool Arm::isConnected() const { return m_impl->isConnected(); }
const fci::arm::DeviceInfo& Arm::deviceInfo() const { return m_impl->getDeviceInfo(); }

} // namespace florid
