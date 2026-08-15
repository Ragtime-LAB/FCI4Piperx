#include "florid/recording/OpenCvRecorder.hpp"

#include "florid/Arm.hpp"
#include "florid/detail/LatencyEstimator.hpp"

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace florid::recording {
namespace {

struct CapturedFrame {
    std::uint64_t index{};
    std::uint64_t host_timestamp_us{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint8_t> bgr;
};

std::uint64_t s_nowUs() {
    return detail::s_nowUs();
}

bool s_isIntegerDevice(const std::string& device) {
    return !device.empty() &&
           std::all_of(device.begin(), device.end(), [](char value) {
               return value >= '0' && value <= '9';
           });
}

} // namespace

struct OpenCvRecorder::Impl {
    Arm& arm;
    const std::size_t queue_capacity;
    CameraConfig config;
    cv::VideoCapture camera;
    std::thread capture_thread;
    std::thread worker_thread;
    std::mutex frame_mutex;
    std::condition_variable frame_cv;
    std::deque<CapturedFrame> frames;
    std::mutex record_mutex;
    std::condition_variable record_cv;
    std::deque<AlignedRecord> records;
    std::deque<TriggerEvent> pending_triggers;
    std::atomic<bool> capture_done{false};
    std::uint64_t next_frame_index{};

    Impl(Arm& arm_ref, std::size_t capacity)
        : arm(arm_ref), queue_capacity(capacity) {}

    void captureLoop(std::atomic<bool>& running) {
        while (running.load()) {
            cv::Mat frame;
            if (!camera.read(frame)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }
            if (frame.empty() || frame.type() != CV_8UC3) {
                if (frame.empty()) continue;
                cv::Mat converted;
                if (frame.channels() == 1) cv::cvtColor(frame, converted, cv::COLOR_GRAY2BGR);
                else if (frame.channels() == 4) cv::cvtColor(frame, converted, cv::COLOR_BGRA2BGR);
                else frame.convertTo(converted, CV_8UC3);
                frame = std::move(converted);
            }

            CapturedFrame captured;
            captured.index = next_frame_index++;
            captured.host_timestamp_us = s_nowUs();
            captured.width = static_cast<std::uint32_t>(frame.cols);
            captured.height = static_cast<std::uint32_t>(frame.rows);
            captured.bgr.assign(frame.data, frame.data + frame.total() * frame.elemSize());
            {
                std::lock_guard<std::mutex> lock(frame_mutex);
                if (frames.size() >= queue_capacity) frames.pop_front();
                frames.push_back(std::move(captured));
            }
            frame_cv.notify_one();
        }
        capture_done = true;
        frame_cv.notify_all();
    }

    void publish(AlignedRecord record, std::atomic<std::uint64_t>& dropped) {
        std::lock_guard<std::mutex> lock(record_mutex);
        if (records.size() >= queue_capacity) {
            ++dropped;
            return;
        }
        records.push_back(std::move(record));
        record_cv.notify_one();
    }

    void workerLoop(std::atomic<bool>& running, std::atomic<std::uint64_t>& dropped) {
        while (running.load() || !capture_done.load() || !pending_triggers.empty()) {
            while (auto trigger = arm.readTriggerOnce()) pending_triggers.push_back(*trigger);
            if (pending_triggers.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (!running.load() && capture_done.load()) break;
                continue;
            }

            std::deque<CapturedFrame> available;
            {
                std::unique_lock<std::mutex> lock(frame_mutex);
                frame_cv.wait_for(lock, std::chrono::milliseconds(2), [&] {
                    return !frames.empty() || capture_done.load() || !running.load();
                });
                available.swap(frames);
            }
            while (!available.empty()) {
                CapturedFrame frame = std::move(available.front());
                available.pop_front();
                if (pending_triggers.empty()) continue;

                auto best = pending_triggers.begin();
                auto best_distance = std::uint64_t(-1);
                for (auto it = pending_triggers.begin(); it != pending_triggers.end(); ++it) {
                    const auto distance = it->receive_host_us > frame.host_timestamp_us
                        ? it->receive_host_us - frame.host_timestamp_us
                        : frame.host_timestamp_us - it->receive_host_us;
                    if (distance < best_distance) {
                        best = it;
                        best_distance = distance;
                    }
                }

                const TriggerEvent trigger = *best;
                pending_triggers.erase(best);
                AlignedRecord record;
                record.camera_slot = config.slot;
                record.trigger_seq = trigger.seq_id;
                record.t_trigger_mcu_us = trigger.timestamp_mcu_us;
                record.t_trigger_host_us = trigger.receive_host_us;
                record.t_frame_host_us = frame.host_timestamp_us;
                record.frame_index = frame.index;
                record.image_width = frame.width;
                record.image_height = frame.height;
                record.image_bgr = std::move(frame.bgr);
                if (auto state = arm.interpolateAt(trigger.timestamp_mcu_us)) {
                    record.state = *state;
                    record.has_state = true;
                }
                publish(std::move(record), dropped);
            }
            if (!running.load() && capture_done.load() && available.empty() &&
                !pending_triggers.empty()) {
                pending_triggers.clear();
            }
        }
    }
};

OpenCvRecorder::OpenCvRecorder(Arm& arm, std::size_t queue_capacity)
    : m_impl(std::make_unique<Impl>(arm, queue_capacity)) {
    if (queue_capacity == 0) throw std::invalid_argument("queue capacity must be non-zero");
}

OpenCvRecorder::~OpenCvRecorder() { stop(); }

void OpenCvRecorder::start(const CameraConfig& config) {
    if (m_running.exchange(true)) throw std::logic_error("recorder already running");
    if (config.slot >= 3) {
        m_running = false;
        throw std::invalid_argument("camera slot must be in [0, 2]");
    }
    m_impl->config = config;
    m_dropped_records = 0;
    m_impl->next_frame_index = 0;
    {
        std::lock_guard<std::mutex> frame_lock(m_impl->frame_mutex);
        m_impl->frames.clear();
    }
    {
        std::lock_guard<std::mutex> record_lock(m_impl->record_mutex);
        m_impl->records.clear();
    }
    m_impl->pending_triggers.clear();
    bool opened = false;
    if (s_isIntegerDevice(config.device)) opened = m_impl->camera.open(std::stoi(config.device));
    else opened = m_impl->camera.open(config.device);
    if (!opened) {
        m_running = false;
        throw std::runtime_error("failed to open OpenCV camera: " + config.device);
    }
    if (config.width) m_impl->camera.set(cv::CAP_PROP_FRAME_WIDTH, config.width);
    if (config.height) m_impl->camera.set(cv::CAP_PROP_FRAME_HEIGHT, config.height);
    if (config.fps) m_impl->camera.set(cv::CAP_PROP_FPS, config.fps);
    m_impl->capture_done = false;
    m_impl->capture_thread = std::thread([this] { m_impl->captureLoop(m_running); });
    m_impl->worker_thread = std::thread([this] { m_impl->workerLoop(m_running, m_dropped_records); });
}

void OpenCvRecorder::stop() {
    if (!m_running.exchange(false)) return;
    m_impl->frame_cv.notify_all();
    if (m_impl->capture_thread.joinable()) m_impl->capture_thread.join();
    m_impl->frame_cv.notify_all();
    if (m_impl->worker_thread.joinable()) m_impl->worker_thread.join();
    m_impl->camera.release();
}

std::optional<AlignedRecord> OpenCvRecorder::readOnce() {
    std::lock_guard<std::mutex> lock(m_impl->record_mutex);
    if (m_impl->records.empty()) return std::nullopt;
    AlignedRecord record = std::move(m_impl->records.front());
    m_impl->records.pop_front();
    return record;
}

} // namespace florid::recording
