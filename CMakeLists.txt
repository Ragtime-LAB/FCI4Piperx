cmake_minimum_required(VERSION 3.20)

project(florid VERSION 0.1.0 LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS   "Build unit tests" OFF)
option(BUILD_EXAMPLES "Build SDK examples" ON)
option(BUILD_PYFLORID "Build Python bindings via pybind11" OFF)
option(BUILD_RECORDING "Build OpenCV recording support" ON)

include(SDKDependencies)

add_subdirectory(protocol)
add_subdirectory(3rdparty/astrial)
add_subdirectory(src)

if(BUILD_RECORDING)
    find_package(OpenCV QUIET COMPONENTS core videoio imgproc)
    if(OpenCV_FOUND)
        add_library(florid_recording STATIC
            src/OpenCvRecorder.cpp
        )
        target_link_libraries(florid_recording
            PUBLIC florid ${OpenCV_LIBS}
        )
        target_include_directories(florid_recording
            PUBLIC include ${OpenCV_INCLUDE_DIRS}
        )
        target_compile_features(florid_recording PUBLIC cxx_std_20)
    else()
        message(WARNING "OpenCV not found; recording support will not be built")
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_PYFLORID)
    find_package(pybind11 REQUIRED)
    find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)
    add_subdirectory(pyflorid4piperx)
endif()
