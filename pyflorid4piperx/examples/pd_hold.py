#!/usr/bin/env python3
"""PD hold example using ActiveControl polling mode (dual arm: 12 joints)."""

import sys
import signal
import numpy as np

sys.path.insert(0, "build/pyflorid4piperx")
from pyflorid4piperx import Arm, JointMIT

g_running = True


def signal_handler(sig, frame):
    global g_running
    g_running = False


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <usb_device>")
        sys.exit(1)

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    uri = f"usb://{sys.argv[1]}"
    print(f"Connecting to {uri} ...")
    arm = Arm.create(uri)
    if arm is None:
        print("Failed to create Arm.")
        sys.exit(1)
    print(f"Connected. fw_dt={arm.firmware_period_us()} us\n")

    ctrl = arm.start_joint_mit_control()

    # Read initial position from first valid frame — no home, no jump
    q_des = np.zeros(12, dtype=np.float32)
    while True:
        state = ctrl.read_once()
        if state.seq != 0:
            q_des[:] = state.q
            break

    print(f"Initial position: {q_des}")
    print("PD hold (kp=10, kd=0.2, firmware gravity). Ctrl+C to stop.\n")

    frame = 0
    while g_running:
        state = ctrl.read_once()
        if state.seq == 0:
            continue

        cmd = JointMIT()
        cmd.q = q_des
        cmd.dq = np.zeros(12, dtype=np.float32)
        cmd.tau = np.zeros(12, dtype=np.float32)
        cmd.kp = np.full(12, 10.0, dtype=np.float32)
        cmd.kd = np.full(12, 0.2, dtype=np.float32)
        cmd.firmware_gravity = True

        ctrl.write_once(cmd)

        frame += 1
        if frame % 500 == 0:
            print(f"  [{frame}] q0={state.q[0]:.3f} q1={state.q[1]:.3f}")

    print(f"Done. Sent {frame} frames.")


if __name__ == "__main__":
    main()
