# fci_protocol

This repository holds the shared protocol definitions used by `Usb2Arm`.

Current scope:
- bootloader control protocol on top of the vendored RPL framing
- packet definitions grouped under `message/`
- stream-session aliases grouped under `session/`
- byte-stream callback adapters grouped under `transport/`

Main header:

```cpp
#include <fci_protocol/protocol.hpp>
```

Current layout:
- `fci_protocol/message/upgrade.hpp`
  - shared upgrade-control packet structs
- `fci_protocol/session/stream_session.hpp`
  - generic stream-session alias over the vendored RPL request/ack machinery
- `fci_protocol/session/upgrade_control_session.hpp`
  - the current upgrade-control message set bound onto a stream session
- `fci_protocol/transport/byte_stream_transport.hpp`
  - small callback-based byte-stream adapters for CDC, TCP, or similar links

Upgrade control notes:
- request packets use RPL request/ack semantics
- data-bearing responses are separate packets carrying the original `req_id`
- this tree now only carries boot status, upgrade-mode switch, and reboot
  control packets; image upload is handled elsewhere

Zephyr module support:
- `zephyr/module.yml` exposes this repository as a standard Zephyr module
- `zephyr/Kconfig` gates the library behind `CONFIG_FCI_PROTOCOL`
- root `CMakeLists.txt` detects `ZEPHYR_BASE` and dispatches to
  `cmake/Zephyr.cmake`; otherwise it behaves like a normal standalone CMake
  subproject

Recommended Zephyr integration boundary:
- this library owns packet definitions, stream-session aliases, and byte-stream
  transport adapters
- the Zephyr app owns USB CDC I/O, protocol mode switching, upgrade policy,
  `UpgradeManager`, slot writes, and reboot policy
- a thin adapter layer should decode protocol requests and forward only the
  product-level actions upward, typically:
  - fill/respond boot status
  - request enter-upgrade mode
  - request reboot
  - pump transport RX/TX bytes between CDC and the protocol transport
