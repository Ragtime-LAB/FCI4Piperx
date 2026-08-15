#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "florid/ControlTypes.hpp"

#include <cstring>

namespace py = pybind11;

static py::array_t<float> s_arr12(const float* s_p) { return py::array_t<float>(12, s_p); }

static void s_set_arr(float* s_dst, py::array s_v, int s_n) {
    std::memcpy(s_dst, s_v.cast<py::array_t<float, py::array::c_style>>().data(), s_n * sizeof(float));
}

void bind_control_types(py::module_& m) {
    // ── JointMIT (dual arm: 12 joints) ──
    py::class_<florid::JointMIT>(m, "JointMIT")
        .def(py::init<>())
        .def_property("q",  [](florid::JointMIT& s) { return s_arr12(s.m_q); },
                           [](florid::JointMIT& s, py::array s_v) { s_set_arr(s.m_q, s_v, 12); })
        .def_property("dq", [](florid::JointMIT& s) { return s_arr12(s.m_dq); },
                           [](florid::JointMIT& s, py::array s_v) { s_set_arr(s.m_dq, s_v, 12); })
        .def_property("tau",[](florid::JointMIT& s) { return s_arr12(s.m_tau); },
                           [](florid::JointMIT& s, py::array s_v) { s_set_arr(s.m_tau, s_v, 12); })
        .def_property("kp", [](florid::JointMIT& s) { return s_arr12(s.m_kp); },
                           [](florid::JointMIT& s, py::array s_v) { s_set_arr(s.m_kp, s_v, 12); })
        .def_property("kd", [](florid::JointMIT& s) { return s_arr12(s.m_kd); },
                           [](florid::JointMIT& s, py::array s_v) { s_set_arr(s.m_kd, s_v, 12); })
        .def_readwrite("firmware_gravity", &florid::JointMIT::m_firmware_gravity)
        .def_readwrite("motion_finished",  &florid::JointMIT::m_motion_finished);
}
