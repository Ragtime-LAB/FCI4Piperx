add_executable(test_transport_pipeline test_transport_pipeline.cpp)

target_link_libraries(test_transport_pipeline
    PRIVATE
        florid
)

target_compile_features(test_transport_pipeline PUBLIC cxx_std_20)

add_test(NAME transport_pipeline COMMAND test_transport_pipeline)

add_executable(test_recording test_recording.cpp)
target_link_libraries(test_recording PRIVATE florid)
target_compile_features(test_recording PUBLIC cxx_std_20)
add_test(NAME recording COMMAND test_recording)
