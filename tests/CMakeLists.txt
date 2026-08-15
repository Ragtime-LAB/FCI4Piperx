add_executable(test_transport_pipeline test_transport_pipeline.cpp)

target_link_libraries(test_transport_pipeline
    PRIVATE
        florid
)

target_compile_features(test_transport_pipeline PUBLIC cxx_std_20)

add_test(NAME transport_pipeline COMMAND test_transport_pipeline)
