#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "florid/core/ActiveControl.hpp"
#include "florid/Gripper.hpp"

namespace py = pybind11;

template <typename T>
void s_bind_active_control(py::module_& m, const char* s_name) {
    py::class_<florid::ActiveControl<T>>(m, s_name)
        .def("read_once",  &florid::ActiveControl<T>::readOnce)
        .def("write_once", &florid::ActiveControl<T>::writeOnce, py::arg("command"));
}

void bind_active_control(py::module_& m) {
    s_bind_active_control<florid::JointMIT>(m,               "ActiveJointMIT");
}

void bind_gripper(py::module_& m) {
    py::class_<florid::Gripper>(m, "Gripper")
        .def("read_once", &florid::Gripper::readOnce)
        .def("start_joint_mit_control", [](florid::Gripper& s_g) {
            return s_g.startJointMITControl();
        });
}
