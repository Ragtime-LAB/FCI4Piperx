#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include "florid/Arm.hpp"
#include "florid/ArmState.hpp"
#include "florid/ArmControl.hpp"
#include "florid/ControlTypes.hpp"
#include "florid/Duration.hpp"
#include "florid/Exceptions.hpp"

#include "fci_protocol/arm/constants.hpp"

namespace py = pybind11;

// ── Sub-module bindings (declared in separate files) ──
void bind_control_types(py::module_& m);
void bind_active_control(py::module_& m);
void bind_gripper(py::module_& m);

PYBIND11_MODULE(_pyflorid, m) {
    m.doc() = "pyflorid4piperx Python bindings — dual-arm MIT control SDK";

    // ── Exceptions ──────────────────────────────────
    py::register_exception<florid::Exception>(m, "Exception", PyExc_RuntimeError);
    py::register_exception<florid::NetworkException>(m, "NetworkException", PyExc_RuntimeError);
    py::register_exception<florid::ControlException>(m, "ControlException", PyExc_RuntimeError);
    py::register_exception<florid::CommandException>(m, "CommandException", PyExc_RuntimeError);
    py::register_exception<florid::InvalidOperationException>(m, "InvalidOperationException", PyExc_RuntimeError);
    py::register_exception<florid::RealtimeException>(m, "RealtimeException", PyExc_RuntimeError);

    // ── Enums ───────────────────────────────────────
    py::enum_<florid::ReconnectPolicy>(m, "ReconnectPolicy")
        .value("Throw", florid::ReconnectPolicy::kThrow)
        .value("Wait",  florid::ReconnectPolicy::kWait);

    py::enum_<florid::ControllerMode>(m, "ControllerMode")
        .value("JointImpedance", florid::ControllerMode::JointImpedance);

    // ── Duration ────────────────────────────────────
    py::class_<florid::Duration>(m, "Duration")
        .def(py::init<>())
        .def("to_sec",   &florid::Duration::toSec)
        .def("to_msec",  &florid::Duration::toMSec)
        .def("to_usec",  &florid::Duration::toUSec)
        .def("__repr__", [](const florid::Duration& s_d) {
            return std::to_string(s_d.toMSec()) + "ms";
        });

    // ── ArmState (dual arm) ─────────────────────────
    py::class_<florid::ArmState>(m, "ArmState")
        .def(py::init<>())
        .def_readwrite("time",               &florid::ArmState::m_time)
        .def_readwrite("seq",                &florid::ArmState::m_seq)
        .def_readwrite("mode",               &florid::ArmState::m_mode)
        .def_readwrite("source_timestamp_us", &florid::ArmState::m_source_timestamp_us)
        .def_readwrite("errors",             &florid::ArmState::m_errors)
        .def_property_readonly("q", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(12, s.m_q);
        })
        .def_property_readonly("dq", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(12, s.m_dq);
        })
        .def_property_readonly("tau", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(12, s.m_tau);
        })
        .def_property_readonly("base_gravity", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(6, s.m_base_gravity);
        })
        .def_property_readonly("O_T_EE", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(32, s.m_O_T_EE);
        })
        .def_property_readonly("F_ext", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(12, s.m_F_ext);
        })
        .def_property_readonly("gripper_q", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(2, s.m_gripper_q);
        })
        .def_property_readonly("gripper_dq", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(2, s.m_gripper_dq);
        })
        .def_property_readonly("gripper_tau", [](const florid::ArmState& s) -> py::array_t<float> {
            return py::array_t<float>(2, s.m_gripper_tau);
        });

    // ── ArmControl ──────────────────────────────────
    py::class_<florid::ArmControl>(m, "ArmControl")
        .def("firmware_period", &florid::ArmControl::firmwarePeriod)
        .def("state_age",       &florid::ArmControl::stateAge)
        .def("estimated_latency", &florid::ArmControl::estimatedLatency)
        .def("receive_jitter_us", &florid::ArmControl::receiveJitterUs)
        .def("receive_hz",      &florid::ArmControl::receiveHz)
        .def("is_reconnecting", &florid::ArmControl::isReconnecting)
        .def("finish_motion",   &florid::ArmControl::finishMotion)
        .def("stop_control",    &florid::ArmControl::stopControl);

    // ── Arm ─────────────────────────────────────────
    py::class_<florid::Arm, std::unique_ptr<florid::Arm>> arm(m, "Arm");
    arm.def_static("create", &florid::Arm::create,
                py::arg("uri"), "Create arm from URI (e.g. 'usb:///dev/ttyACM1')");
    arm.def("enable",            &florid::Arm::enable);
    arm.def("drag",              &florid::Arm::drag);
    arm.def("disable",           &florid::Arm::disable);
    arm.def("read_once",         &florid::Arm::readOnce);
    arm.def("firmware_period_us", &florid::Arm::firmwarePeriodUs);
    arm.def("reconnect_policy",  &florid::Arm::reconnectPolicy);
    arm.def("set_reconnect_policy", &florid::Arm::setReconnectPolicy);
    arm.def("is_connected",      &florid::Arm::isConnected);
    arm.def("stop",              &florid::Arm::stop);
    arm.def("gripper", &florid::Arm::gripper, py::return_value_policy::reference);
    arm.def("start_joint_mit_control", &florid::Arm::startJointMITControl);

    // ── Sub-modules ─────────────────────────────────
    bind_control_types(m);
    bind_active_control(m);
    bind_gripper(m);
}
