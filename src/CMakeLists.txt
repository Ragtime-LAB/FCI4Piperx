add_library(florid
    tick_std.cpp
    timestamp_std.cpp
    TickProvider.cpp
    AstrialUSBTransport.cpp
    ArmImpl.cpp
    ArmCore.cpp
    Arm.cpp
    GripperCore.cpp
    Gripper.cpp
    LatencyEstimator.cpp
)

target_link_libraries(florid
    PUBLIC
        fci_protocol
        astrial
)

target_include_directories(florid
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

target_compile_features(florid PUBLIC cxx_std_20)
