#include "florid/Arm.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>

static std::atomic<bool> g_running{true};

void s_signalHandler(int) { g_running = false; }

void s_printUsage(const char* s_prog) {
    fprintf(stderr, "Usage: %s <usb_device>\n", s_prog);
    exit(1);
}

int main(int s_argc, char** s_argv) {
    if (s_argc < 2) s_printUsage(s_argv[0]);

    std::string s_uri = "usb://";
    s_uri += s_argv[1];

    signal(SIGINT, s_signalHandler);
    signal(SIGTERM, s_signalHandler);

    printf("Connecting to %s ...\n", s_uri.c_str());
    auto s_arm = florid::Arm::create(s_uri);
    if (!s_arm) {
        fprintf(stderr, "Failed to create Arm.\n");
        return 1;
    }
    printf("Connected. fw_dt=%u us\n\n", s_arm->firmwarePeriodUs());

    // Control loop: sinusoidal motion per joint
    printf("Starting joint sine motion on joint 1 (kp=10.0, kd=0.2, 0→+0.3 rad)\n");

    auto s_start_time = std::chrono::steady_clock::now();
    int s_frame_count = 0;

    s_arm->control([&](const florid::ArmState&,
                        florid::ArmControl& s_ctrl) -> florid::JointMIT
    {
        constexpr float g_kp = 10.0f;
        constexpr float g_kd = 0.2f;

        auto s_now = std::chrono::steady_clock::now();
        double s_t = std::chrono::duration<double>(s_now - s_start_time).count();
        if (s_t < 0.0) s_t = 0.0;

        // ── Latency log every 2 seconds ──
        if (s_frame_count % 1000 == 0 && s_frame_count > 0) {
            printf("  [t=%.1fs] age=%.2fms  rtt=%.2fms  jitter=%.0fus  rxHz=%.1f\n",
                   s_t,
                   s_ctrl.stateAge().toMSec(),
                   s_ctrl.estimatedLatency().toMSec(),
                   s_ctrl.receiveJitterUs(),
                   s_ctrl.receiveHz());
        }
        s_frame_count++;

        florid::JointMIT s_cmd;
        s_cmd.m_firmware_gravity = true; // firmware computes gravity

        for (int s_i = 0; s_i < 12; ++s_i) {
            float s_target = 0.0f;
            if (s_i == 1) {
                s_target = 0.3f * static_cast<float>(0.5 - 0.5 * std::cos(s_t * 1.5));
                if (s_target < 0.0f) s_target = 0.0f;
                if (s_target > 0.3f) s_target = 0.3f;
            }

            s_cmd.m_q[s_i]  = static_cast<float>(s_target);
            s_cmd.m_dq[s_i]  = 0.0f;
            s_cmd.m_tau[s_i] = 0.0f;  // zero feedforward, firmware adds gravity
            s_cmd.m_kp[s_i]  = g_kp;
            s_cmd.m_kd[s_i]  = g_kd;
        }

        if (!g_running) {
            return florid::JointMIT::MotionFinished(s_cmd);
        }

        return s_cmd;
    });

    printf("Control loop ended.\n");
    return 0;
}
