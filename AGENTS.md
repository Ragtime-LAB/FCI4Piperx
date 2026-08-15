# AGENTS.md

SDK for a **dual Piperx arm** (single `Arm` object driving 2 arms = 12 joints + 2 grippers).
This is a stripped fork of `libflorid`, now diverged heavily — trust this repo's headers,
not upstream libflorid.

## Build & test

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure      # single test: transport_pipeline
```

- Options default **OFF**: `BUILD_TESTS`, `BUILD_PYFLORID`; `BUILD_EXAMPLES` defaults ON. Binding:
  ```sh
  cmake -B build -DBUILD_PYFLORID=ON ... && cmake --build build
  ```
- Library target is `florid`; C++20 required. On this env Python 3.14 + pybind11 3.1 are installed.
- `.gitignore` already excludes `build*`, `*.cpython*`, `__pycache__`, `backup`, `.cache`.

## Architecture (non-obvious)

- **`Arm` is dual-arm**: `Arm::s_kNumJoints == 12` (0–5 = ARM1, 6–11 = ARM2), `ArmState` has
  `m_q[12]`, per-arm `m_base_gravity[6]`/pose/F_ext, and `m_gripper_q[2]`. `GripperState::m_q[2]`
  maps index 0→ARM1, 1→ARM2 gripper.
- **Only `JointMIT` control exists.** POS/VEL/PVT/Cartesian, Model/kinematics, MPC, MotorRegisters,
  DeviceSettings, diagnostics, and `home()` were all removed. `core/traits.hpp`
  `is_control_command` is specialized only for `JointMIT` — new control types will not compile
  without adding a specialization and their packet path in `detail/ArmImpl.hpp`.
- **Arm mode via `SetArmModeRequest`**: `Arm::enable`→`Pc`, `drag`→`Drag`, `disable`→`Damp`.
  These act on **both arms** (no per-arm selection).
- Clock source is `detail/MonotonicTickProvider` (`tick_std.cpp`); transport is `io_uring` over
  liburing when the kernel ≥ 5.15 (auto-detected), else epoll.

## Defaults that must stay in sync

- Firmware period is **250 Hz**, hardcoded in **3 places** — change all together:
  `include/florid/detail/ArmImpl.hpp` (`m_fw_dt_us{4000}`),
  `src/ArmImpl.cpp` (`ArmControl::firmwarePeriod()` fallback `: 4000`),
  `tests/test_transport_pipeline.cpp` (assert `== 4000`).

## Python bindings (`pyflorid4piperx/`)

- **Naming quirk**: pybind11 module is still `_pyflorid` (see `__init__.py` `from _pyflorid import *`),
  but the python package/import path is `pyflorid4piperx`. Import as
  `from pyflorid4piperx import Arm, JointMIT`. Examples insert `build/pyflorid4piperx` in `sys.path`.
- Binding source mirrors the C++ API cutdown (only JointMIT / ActiveJointMIT / dual-arm state).

## Vendored protocol

- `protocol/` is the **frozen dual-arm fci_protocol** (12 joints, 2 grippers). Treat as read-only
  vendor; SDK links `fci_protocol` and consumes
  `SetArmModeRequestPacket`/`ArmStatus`/`GripperStatus`/`GetDeviceInfoResponsePacket` etc.
  Don't regenerate or "fix" packets unless the wire format is actually changing.

## Conventions

- All public SDK symbols use `florid::` namespace and `s_` prefix on anonymous-namespace helpers
  (`s_convertStatus`, `s_requestPcMode`, `s_fetchDeviceInfo`) — keep that style in new code.
- `examples/` uses a `add_florid_example(name src)` helper; only the 3 current examples are registered.
