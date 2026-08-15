add_library(rpl INTERFACE)
add_library(rpl::rpl ALIAS rpl)

target_include_directories(rpl INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(rpl INTERFACE cxx_std_20)

# 环境检测：在 Linux 环境下默认使用 std::atomic 保证多核安全
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(RPL_USE_STD_ATOMIC_DEFAULT ON)
else()
    set(RPL_USE_STD_ATOMIC_DEFAULT OFF)
endif()

option(RPL_USE_STD_ATOMIC "Use std::atomic for SeqLock (recommended for SMP/Multi-core)" ${RPL_USE_STD_ATOMIC_DEFAULT})

if(RPL_USE_STD_ATOMIC)
    target_compile_definitions(rpl INTERFACE RPL_USE_STD_ATOMIC)
endif()

target_compile_definitions(rpl INTERFACE
        FRAME_HEADER_TAG=${FRAME_HEADER_TAG}
)
