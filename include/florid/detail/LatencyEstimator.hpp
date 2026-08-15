#ifndef FLORID_DETAIL_LATENCY_ESTIMATOR_HPP
#define FLORID_DETAIL_LATENCY_ESTIMATOR_HPP

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>

namespace florid::detail {

inline std::uint64_t s_nowUs() {
    static const auto s_epoch = std::chrono::steady_clock::now();
    auto s_now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(s_now - s_epoch).count();
}

class LatencyEstimator {
public:
    LatencyEstimator() = default;

    void markReceived(std::uint64_t s_echo_timestamp_us, std::uint64_t s_host_now_us);

    void markSent(std::uint64_t s_host_now_us);

    std::uint64_t stateAgeUs(std::uint64_t s_host_now_us) const {
        if (m_recv_time_us == 0) return 0;
        return s_host_now_us - m_recv_time_us;
    }

    double estimatedLatencyMs() const { return m_smoothed_rtt_ms; }

    double receiveJitterUs() const { return m_jitter_us; }

    double receiveHz(std::uint64_t s_host_now_us) const;

    void setEmaAlpha(float s_alpha) { m_alpha = s_alpha; }

private:
    static constexpr int g_window_size = 64;

    std::uint64_t m_recv_time_us{0};
    std::uint64_t m_prev_recv_time_us{0};
    std::array<std::uint64_t, g_window_size> m_intervals{};
    int m_window_idx{0};
    int m_window_count{0};
    double m_smoothed_rtt_ms{0.0};
    double m_jitter_us{0.0};
    bool m_has_valid_rtt{false};
    float m_alpha{0.1f};
};

} // namespace florid::detail

#endif // FLORID_DETAIL_LATENCY_ESTIMATOR_HPP
