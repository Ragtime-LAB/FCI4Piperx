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
