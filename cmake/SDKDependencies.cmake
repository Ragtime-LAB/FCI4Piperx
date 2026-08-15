# readerwriterqueue is vendored in 3rdparty/ — add_subdirectory called from top-level
# ASIO / Ethernet transport is deferred; USB transport uses Astrial from 3rdparty/astrial
# pybind11 is found via find_package in top-level CMakeLists.txt
# protocol is a git submodule at protocol/ — add_subdirectory called from top-level
