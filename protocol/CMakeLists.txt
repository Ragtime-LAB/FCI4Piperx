cmake_minimum_required(VERSION 3.20.0)

# App-local copy of the FCI protocol (header-only) plus the vendored RPL
# headers. Exposed to the app as an INTERFACE library so include paths, the
# FRAME_HEADER_TAG define and C++20 propagate through target_link_libraries.
#
# This is NOT the west-managed module: the app keeps CONFIG_FCI_PROTOCOL=n so
# only this copy is compiled.

add_library(fci_protocol INTERFACE)

target_include_directories(fci_protocol INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/rpl/include
)

target_compile_definitions(fci_protocol INTERFACE FRAME_HEADER_TAG=0xA5)
target_compile_features(fci_protocol INTERFACE cxx_std_20)
