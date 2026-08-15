#ifndef FLORID_DURATION_HPP
#define FLORID_DURATION_HPP

#include <cstdint>

namespace florid {

class Duration {
public:
    constexpr Duration() : m_duration_ms(0) {}
    static constexpr Duration fromMSec(std::uint64_t ms) {
        Duration s_d;
        s_d.m_duration_ms = ms;
        return s_d;
    }
    static constexpr Duration fromUSec(std::uint64_t us) {
        Duration s_d;
        s_d.m_duration_ms = static_cast<double>(us) / 1000.0;
        return s_d;
    }
    static constexpr Duration fromSec(double sec) {
        Duration s_d;
        s_d.m_duration_ms = sec * 1000.0;
        return s_d;
    }

    constexpr double toSec() const { return m_duration_ms / 1000.0; }
    constexpr double toMSec() const { return m_duration_ms; }
    constexpr std::uint64_t toUSec() const { return static_cast<std::uint64_t>(m_duration_ms * 1000.0); }

    Duration operator+(const Duration& s_rhs) const { return fromMSec(m_duration_ms + s_rhs.m_duration_ms); }
    Duration operator-(const Duration& s_rhs) const { return fromMSec(m_duration_ms - s_rhs.m_duration_ms); }
    Duration& operator+=(const Duration& s_rhs) { m_duration_ms += s_rhs.m_duration_ms; return *this; }
    Duration& operator-=(const Duration& s_rhs) { m_duration_ms -= s_rhs.m_duration_ms; return *this; }
    bool operator==(const Duration& s_rhs) const { return m_duration_ms == s_rhs.m_duration_ms; }
    bool operator!=(const Duration& s_rhs) const { return m_duration_ms != s_rhs.m_duration_ms; }
    bool operator<(const Duration& s_rhs) const { return m_duration_ms < s_rhs.m_duration_ms; }
    bool operator>(const Duration& s_rhs) const { return m_duration_ms > s_rhs.m_duration_ms; }
    bool operator<=(const Duration& s_rhs) const { return m_duration_ms <= s_rhs.m_duration_ms; }
    bool operator>=(const Duration& s_rhs) const { return m_duration_ms >= s_rhs.m_duration_ms; }

private:
    double m_duration_ms{0.0};
};

} // namespace florid

#endif // FLORID_DURATION_HPP
