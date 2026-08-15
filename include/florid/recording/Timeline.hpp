#ifndef FLORID_RECORDING_TIMELINE_HPP
#define FLORID_RECORDING_TIMELINE_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <mutex>
#include <optional>

namespace florid::recording {

struct TriggerEvent {
    std::uint16_t seq_id{};
    std::uint64_t timestamp_mcu_us{};
    std::uint64_t receive_host_us{};
};

struct TimedArmSample {
    std::uint64_t timestamp_mcu_us{};
    std::uint32_t seq{};
    std::array<float, 12> q{};
    std::array<float, 12> dq{};
    std::array<float, 2> gripper_q{};
};

struct InterpolatedState {
    std::uint64_t timestamp_mcu_us{};
    std::uint64_t sample_before_us{};
    std::uint64_t sample_after_us{};
    std::uint32_t sample_before_seq{};
    std::uint32_t sample_after_seq{};
    float alpha{};
    std::array<float, 12> q{};
    std::array<float, 12> dq{};
    std::array<float, 2> gripper_q{};

    enum class Status : std::uint8_t {
        Exact,
        Interpolated,
        Gap,
        OutOfRange,
    } status{Status::OutOfRange};
};

class AngleTimeline {
public:
    explicit AngleTimeline(std::size_t capacity = 2048,
                           std::uint64_t max_gap_us = 20000)
        : m_capacity(capacity), m_max_gap_us(max_gap_us) {}

    void push(const TimedArmSample& sample) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_capacity == 0) return;
        if (!m_samples.empty() && sample.timestamp_mcu_us <= m_samples.back().timestamp_mcu_us) {
            return;
        }
        m_samples.push_back(sample);
        while (m_samples.size() > m_capacity) m_samples.pop_front();
    }

    std::optional<InterpolatedState> interpolate(std::uint64_t timestamp_mcu_us) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_samples.empty() || timestamp_mcu_us < m_samples.front().timestamp_mcu_us ||
            timestamp_mcu_us > m_samples.back().timestamp_mcu_us) {
            return std::nullopt;
        }

        auto after = m_samples.begin();
        while (after != m_samples.end() && after->timestamp_mcu_us < timestamp_mcu_us) ++after;
        if (after == m_samples.begin()) return makeExact(*after, timestamp_mcu_us);
        if (after == m_samples.end()) return makeExact(m_samples.back(), timestamp_mcu_us);

        const auto& before = *std::prev(after);
        const auto& after_sample = *after;
        const auto gap = after_sample.timestamp_mcu_us - before.timestamp_mcu_us;
        if (gap == 0 || timestamp_mcu_us == before.timestamp_mcu_us)
            return makeExact(before, timestamp_mcu_us);
        if (timestamp_mcu_us == after_sample.timestamp_mcu_us)
            return makeExact(after_sample, timestamp_mcu_us);

        InterpolatedState result;
        result.timestamp_mcu_us = timestamp_mcu_us;
        result.sample_before_us = before.timestamp_mcu_us;
        result.sample_after_us = after_sample.timestamp_mcu_us;
        result.sample_before_seq = before.seq;
        result.sample_after_seq = after_sample.seq;
        result.alpha = static_cast<float>(timestamp_mcu_us - before.timestamp_mcu_us) /
                       static_cast<float>(gap);
        result.status = gap > m_max_gap_us ? InterpolatedState::Status::Gap
                                           : InterpolatedState::Status::Interpolated;
        for (std::size_t i = 0; i < result.q.size(); ++i) {
            result.q[i] = before.q[i] + result.alpha * (after_sample.q[i] - before.q[i]);
            result.dq[i] = before.dq[i] + result.alpha * (after_sample.dq[i] - before.dq[i]);
        }
        for (std::size_t i = 0; i < result.gripper_q.size(); ++i) {
            result.gripper_q[i] = before.gripper_q[i] +
                                  result.alpha * (after_sample.gripper_q[i] - before.gripper_q[i]);
        }
        return result;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_samples.clear();
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_samples.size();
    }

private:
    static InterpolatedState makeExact(const TimedArmSample& sample,
                                       std::uint64_t timestamp_mcu_us) {
        InterpolatedState result;
        result.timestamp_mcu_us = timestamp_mcu_us;
        result.sample_before_us = sample.timestamp_mcu_us;
        result.sample_after_us = sample.timestamp_mcu_us;
        result.sample_before_seq = sample.seq;
        result.sample_after_seq = sample.seq;
        result.q = sample.q;
        result.dq = sample.dq;
        result.gripper_q = sample.gripper_q;
        result.status = InterpolatedState::Status::Exact;
        return result;
    }

    std::size_t m_capacity;
    std::uint64_t m_max_gap_us;
    mutable std::mutex m_mutex;
    std::deque<TimedArmSample> m_samples;
};

} // namespace florid::recording

#endif // FLORID_RECORDING_TIMELINE_HPP
