cmake_minimum_required(VERSION 3.20)

function(add_florid_example s_name s_src)
    add_executable(florid_example_${s_name} ${s_src})
    target_link_libraries(florid_example_${s_name} PRIVATE florid)
    target_compile_features(florid_example_${s_name} PRIVATE cxx_std_20)
endfunction()

add_florid_example(00_echo_arm_state   00_echo_arm_state.cpp)
add_florid_example(01_joint_sine_motion 01_joint_sine_motion.cpp)
add_florid_example(01_gripper_move      01_gripper_move.cpp)
