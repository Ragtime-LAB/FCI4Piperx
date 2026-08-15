pybind11_add_module(_pyflorid
    src/pyflorid.cpp
    src/control_types.cpp
    src/active_control.cpp
)

target_link_libraries(_pyflorid PRIVATE florid)

if(TARGET florid_recording)
    target_link_libraries(_pyflorid PRIVATE florid_recording)
    target_compile_definitions(_pyflorid PRIVATE FLORID_HAS_RECORDING=1)
endif()

target_include_directories(_pyflorid PRIVATE
    "${Python_NumPy_INCLUDE_DIRS}"
)

target_compile_features(_pyflorid PRIVATE cxx_std_20)

install(TARGETS _pyflorid
    LIBRARY DESTINATION pyflorid4piperx
    RUNTIME DESTINATION pyflorid4piperx
)
