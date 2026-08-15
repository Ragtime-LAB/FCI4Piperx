#ifndef FLORID_ERRORS_HPP
#define FLORID_ERRORS_HPP

#include <cstdint>

namespace florid {

struct Errors {
    std::uint32_t m_bits{0};

    explicit operator bool() const { return m_bits != 0; }

    bool jointPositionLimitsViolation() const { return (m_bits & (1u << 0)) != 0; }
    bool cartesianPositionLimitsViolation() const { return (m_bits & (1u << 1)) != 0; }
    bool selfCollision() const { return (m_bits & (1u << 2)) != 0; }
    bool jointVelocityViolation() const { return (m_bits & (1u << 3)) != 0; }
    bool cartesianVelocityViolation() const { return (m_bits & (1u << 4)) != 0; }
    bool forceControlSafetyViolation() const { return (m_bits & (1u << 5)) != 0; }
    bool jointReflex() const { return (m_bits & (1u << 6)) != 0; }
    bool cartesianReflex() const { return (m_bits & (1u << 7)) != 0; }
    bool communicationConstraintsViolation() const { return (m_bits & (1u << 8)) != 0; }
    bool emergencyStop() const { return (m_bits & (1u << 9)) != 0; }
    bool watchdogTimeout() const { return (m_bits & (1u << 10)) != 0; }
};

} // namespace florid

#endif // FLORID_ERRORS_HPP
