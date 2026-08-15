#include "florid/Arm.hpp"
#include "florid/detail/AstrialUSBTransport.hpp"

#include <atomic>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

static std::atomic<bool> g_running{true};

struct s_EndEffectorPose {
    double x, y, z, rx, ry, rz;
};

// Column-major 4x4 homogenous transform, per arm 16 floats.
s_EndEffectorPose s_poseFromMatrix(const float* m) {
    s_EndEffectorPose p;
    p.x  = m[12];
    p.y  = m[13];
    p.z  = m[14];
    p.rx = std::atan2(m[6], m[10]);
    p.ry = std::atan2(-m[2], std::sqrt(m[6] * m[6] + m[10] * m[10]));
    p.rz = std::atan2(m[4], m[0]);
    return p;
}

static const char* s_modeName(std::uint32_t s_mode) {
    switch (static_cast<fci::arm::ArmMode>(s_mode)) {
        case fci::arm::ArmMode::Pc:  return "PC";
        case fci::arm::ArmMode::Drag: return "DRAG";
        case fci::arm::ArmMode::Damp: return "DAMP";
        default: return "??";
    }
}

void s_signalHandler(int) {
    g_running = false;
}

void s_printUsage(const char* s_prog) {
    fprintf(stderr, "Usage: %s <usb_device>  (e.g. %s /dev/ttyACM0)\n", s_prog, s_prog);
    exit(1);
}

int main(int s_argc, char** s_argv) {
    if (s_argc < 2) s_printUsage(s_argv[0]);

    std::string s_uri = "usb://";
    s_uri += s_argv[1];

    signal(SIGINT, s_signalHandler);
    signal(SIGTERM, s_signalHandler);

    // ── List available USB devices ──
    printf("=== USB Devices ===\n");
    auto s_devices = florid::AstrialUSBTransport::listDevices();
    for (const auto& s_d : s_devices) {
        printf("  %-20s %04X:%04X  %s\n",
               s_d.m_port_name.c_str(), s_d.m_vendor_id, s_d.m_product_id,
               s_d.m_description.c_str());
    }
    printf("\n");

    // ── Connect ──
    printf("Connecting to %s ...\n", s_uri.c_str());
    auto s_arm = florid::Arm::create(s_uri);
    if (!s_arm) {
        fprintf(stderr, "Failed to create Arm (unknown URI scheme).\n");
        return 1;
    }

    printf("Connected. Firmware period: %u us (%.1f Hz)\n",
           s_arm->firmwarePeriodUs(),
           1e6 / s_arm->firmwarePeriodUs());

    // ── Device info ──
    const auto& s_info = s_arm->deviceInfo();
    printf("Device info:\n");
    printf("  Board:        %s\n", s_info.board_name.data());
    printf("  Custom name:  %s\n", s_info.custom_name.data());
    printf("  FW version:   %u.%u.%u\n",
           s_info.fw_version.major, s_info.fw_version.minor, s_info.fw_version.patch);
    printf("  Protocol ver: %u.%u.%u\n",
           s_info.protocol_version.major, s_info.protocol_version.minor, s_info.protocol_version.patch);
    printf("  FW type:      %d\n", s_info.fw_type);
    printf("\n");

    // ── Echo state ──
    printf("\n=== Arm State Stream ===\n");
    printf(" seq  | ARM1: q0..q5         | ARM2: q6..q11        |  mode  | errs\n");
    printf("------|-----------------------|----------------------|--------|------\n");

    s_arm->read([&](const florid::ArmState& s_state) {
        if (s_state.m_seq == 0) return g_running.load();

        printf("%5u | %+6.3f %+6.3f %+6.3f %+6.3f %+6.3f %+6.3f |"
               " %+6.3f %+6.3f %+6.3f %+6.3f %+6.3f %+6.3f |"
               " %-6s | 0x%02X\n",
               s_state.m_seq,
               s_state.m_q[0], s_state.m_q[1], s_state.m_q[2],
               s_state.m_q[3], s_state.m_q[4], s_state.m_q[5],
               s_state.m_q[6], s_state.m_q[7], s_state.m_q[8],
               s_state.m_q[9], s_state.m_q[10], s_state.m_q[11],
               s_modeName(s_state.m_mode), s_state.m_errors);

        const s_EndEffectorPose s_p1 = s_poseFromMatrix(&s_state.m_O_T_EE[0]);
        const s_EndEffectorPose s_p2 = s_poseFromMatrix(&s_state.m_O_T_EE[16]);
        printf("   P1: x %+7.3f y %+7.3f z %+7.3f | RPY %+7.3f %+7.3f %+7.3f |"
               " P2: x %+7.3f y %+7.3f z %+7.3f | RPY %+7.3f %+7.3f %+7.3f\n",
               s_p1.x, s_p1.y, s_p1.z, s_p1.rx, s_p1.ry, s_p1.rz,
               s_p2.x, s_p2.y, s_p2.z, s_p2.rx, s_p2.ry, s_p2.rz);

        return g_running.load();
    });

    printf("\nStopped (signal received).\n");
    return 0;
}
