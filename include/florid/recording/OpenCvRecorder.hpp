#ifndef FLORID_RECORDING_OPENCV_RECORDER_HPP
#define FLORID_RECORDING_OPENCV_RECORDER_HPP

#include "florid/recording/Timeline.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace florid {
class Arm;

namespace recording {

struct CameraConfig {
    std::uint32_t slot{};
    std::string device{"0"};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint32_t fps{};
    bool hardware_trigger{};
};

struct AlignedRecord {
    std::uint32_t camera_slot{};
    std::uint64_t trigger_seq{};
    std::uint64_t t_trigger_mcu_us{};
    std::uint64_t t_trigger_host_us{};
    std::uint64_t t_frame_host_us{};
    std::uint64_t frame_index{};
    std::uint32_t image_width{};
    std::uint32_t image_height{};
    std::vector<std::uint8_t> image_bgr;
    InterpolatedState state{};
    bool has_state{};
};

class OpenCvRecorder {
public:
    explicit OpenCvRecorder(Arm& arm, std::size_t queue_capacity = 64);
    ~OpenCvRecorder();

    OpenCvRecorder(const OpenCvRecorder&) = delete;
    OpenCvRecorder& operator=(const OpenCvRecorder&) = delete;

    void start(const CameraConfig& config);
    void stop();
    bool running() const noexcept { return m_running.load(); }

    std::optional<AlignedRecord> readOnce();
    std::uint64_t droppedRecords() const noexcept { return m_dropped_records.load(); }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    std::atomic<bool> m_running{false};
    std::atomic<std::uint64_t> m_dropped_records{0};
};

} // namespace recording
} // namespace florid

#endif // FLORID_RECORDING_OPENCV_RECORDER_HPP
