#include "florid/detail/LatencyEstimator.hpp"

#include <algorithm>
#include <numeric>

namespace florid::detail {

void LatencyEstimator::markReceived(std::uint64_t s_echo_timestamp_us,
                                     std::uint64_t s_host_now_us) {
    // ── stateAge ──
    m_recv_time_us = s_host_now_us;

    // ── frame interval jitter (sliding window) ──
    if (m_prev_recv_time_us > 0) {
        std::uint64_t s_delta = s_host_now_us - m_prev_recv_time_us;
        m_intervals[m_window_idx] = s_delta;
        m_window_idx = (m_window_idx + 1) % g_window_size;
        if (m_window_count < g_window_size) m_window_count++;

        if (m_window_count >= 4) {
            // Mean
            double s_sum = 0.0;
            for (int s_i = 0; s_i < m_window_count; ++s_i)
                s_sum += static_cast<double>(m_intervals[s_i]);
            double s_mean = s_sum / m_window_count;

            // Standard deviation
            double s_var = 0.0;
            for (int s_i = 0; s_i < m_window_count; ++s_i) {
                double s_d = static_cast<double>(m_intervals[s_i]) - s_mean;
                s_var += s_d * s_d;
            }
            m_jitter_us = std::sqrt(s_var / m_window_count);
        }
    }
    m_prev_recv_time_us = s_host_now_us;

    // ── RTT from echo timestamp ──
    if (s_echo_timestamp_us > 0) {
        double s_rtt_ms = static_cast<double>(s_host_now_us - s_echo_timestamp_us) / 1000.0;

        if (!m_has_valid_rtt) {
            m_smoothed_rtt_ms = s_rtt_ms;
            m_has_valid_rtt = true;
        } else {
            m_smoothed_rtt_ms = m_alpha * s_rtt_ms + (1.0f - m_alpha) * m_smoothed_rtt_ms;
        }
    }
}

double LatencyEstimator::receiveHz(std::uint64_t s_host_now_us) const {
    if (m_window_count < 4) return 0.0;
    std::uint64_t s_sum = 0;
    for (int s_i = 0; s_i < m_window_count; ++s_i)
        s_sum += m_intervals[s_i];
    double s_avg_us = static_cast<double>(s_sum) / m_window_count;
    if (s_avg_us <= 0.0) return 0.0;
    return 1e6 / s_avg_us;
}

void LatencyEstimator::markSent(std::uint64_t /*s_host_now_us*/) {}

} // namespace florid::detail
