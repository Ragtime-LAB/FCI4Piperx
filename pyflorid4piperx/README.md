# pyflorid4piperx

Python bindings for the dual-PiperX FCI SDK. One `Arm` instance exposes both
six-joint arms as a 12-joint state and command interface.

Build and install from the SDK repository with:

```bash
python -m pip install ./pyflorid4piperx
```

Then import the bindings with:

```python
from pyflorid4piperx import Arm, JointMIT
```

The optional OpenCV recorder is included when CMake finds OpenCV and
`BUILD_RECORDING` is enabled.

Read-only clients that must not change firmware mode on shutdown should call:

```python
arm.set_disconnect_mode_change_enabled(False)
```

The default remains `True` so control clients retain the existing best-effort
transition to Damp when the SDK object is destroyed.
