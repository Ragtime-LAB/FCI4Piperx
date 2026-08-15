#include "florid/Arm.hpp"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>

static std::atomic<bool> g_running{true};

void s_signalHandler(int) { g_running = false; }

void s_printUsage(const char *s_prog) {
  fprintf(stderr, "Usage: %s <usb_device>\n", s_prog);
  exit(1);
}

int main(int s_argc, char **s_argv) {
  if (s_argc < 2)
    s_printUsage(s_argv[0]);

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

  // ── Gripper setup ──
  auto &s_gripper = s_arm->gripper();
  auto s_ctrl = s_gripper.startJointMITControl();

  // Read initial positions (ARM1 gripper = [0], ARM2 gripper = [1])
  float s_current[2] = {0.0f, 0.0f};
  for (int s_i = 0; s_i < 50; ++s_i) {
    auto s_state = s_ctrl->readOnce();
    if (s_state.m_seq != 0) {
      s_current[0] = s_state.m_gripper_q[0];
      s_current[1] = s_state.m_gripper_q[1];
      break;
    }
  }
  printf("Initial gripper positions: [%.4f, %.4f]\n\n", s_current[0], s_current[1]);

  // ── Move loop: open ↔ close ──
  float s_target[2] = {s_current[0], s_current[1]};
  float s_open[2]   = {s_current[0] - 0.5f, s_current[1] - 0.5f};
  float s_close[2]  = {s_current[0], s_current[1]};
  int s_phase_frames = 0;
  int s_frame = 0;
  bool s_is_open = false;

  printf("Gripper MIT control (kp=20, kd=0.5). Cycling both grippers open<->close.\n");

  while (g_running) {
    auto s_state = s_ctrl->readOnce();
    if (s_state.m_seq == 0)
      continue;

    s_phase_frames++;

    if (s_phase_frames > 1000) {
      s_phase_frames = 0;
      s_is_open = !s_is_open;
      for (int s_i = 0; s_i < 2; ++s_i)
        s_target[s_i] = s_is_open ? s_open[s_i] : s_close[s_i];
      printf("  -> target=[%.4f, %.4f]\n", s_target[0], s_target[1]);
    }

    florid::JointMIT s_cmd;
    for (int s_i = 0; s_i < 2; ++s_i) {
      s_cmd.m_q[s_i]   = s_target[s_i];
      s_cmd.m_dq[s_i]  = 0.0f;
      s_cmd.m_tau[s_i] = 0.0f;
      s_cmd.m_kp[s_i]  = 20.0f;
      s_cmd.m_kd[s_i]  = 0.5f;
    }

    s_ctrl->writeOnce(s_cmd);

    if (++s_frame % 500 == 0) {
      printf("  [%d] gripper_q=[%.4f, %.4f]  target=[%.4f, %.4f]\n", s_frame,
             s_state.m_gripper_q[0], s_state.m_gripper_q[1],
             s_target[0], s_target[1]);
    }
  }

  printf("Done. Sent %d frames.\n", s_frame);
  return 0;
}
