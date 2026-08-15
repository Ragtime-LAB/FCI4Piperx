#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>

#include <cstring>

#include "florid/Arm.hpp"
#include "florid/ArmState.hpp"
#include "florid/ArmControl.hpp"
#include "florid/ControlTypes.hpp"
#include "florid/Duration.hpp"
#include "florid/Exceptions.hpp"

#include "fci_protocol/arm/constants.hpp"

#ifdef FLORID_HAS_RECORDING
#include "florid/recording/OpenCvRecorder.hpp"
#endif

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
#ifdef FLORID_HAS_RECORDING
    arm.def("read_trigger_once", &florid::Arm::readTriggerOnce);
    arm.def("interpolate_at",    &florid::Arm::interpolateAt);
#endif
    arm.def("firmware_period_us", &florid::Arm::firmwarePeriodUs);
    arm.def("reconnect_policy",  &florid::Arm::reconnectPolicy);
    arm.def("set_reconnect_policy", &florid::Arm::setReconnectPolicy);
    arm.def("is_connected",      &florid::Arm::isConnected);
    arm.def("disconnect_mode_change_enabled", &florid::Arm::disconnectModeChangeEnabled);
    arm.def("set_disconnect_mode_change_enabled", &florid::Arm::setDisconnectModeChangeEnabled,
            py::arg("enabled"),
            "When false, destroying this Arm leaves the firmware mode unchanged.");
    arm.def("stop",              &florid::Arm::stop);
    arm.def("gripper", &florid::Arm::gripper, py::return_value_policy::reference);
    arm.def("start_joint_mit_control", &florid::Arm::startJointMITControl);

    // ── Sub-modules ─────────────────────────────────
    bind_control_types(m);
    bind_active_control(m);
    bind_gripper(m);

#ifdef FLORID_HAS_RECORDING
    py::enum_<florid::recording::InterpolatedState::Status>(m, "InterpolationStatus")
        .value("Exact", florid::recording::InterpolatedState::Status::Exact)
        .value("Interpolated", florid::recording::InterpolatedState::Status::Interpolated)
        .value("Gap", florid::recording::InterpolatedState::Status::Gap)
        .value("OutOfRange", florid::recording::InterpolatedState::Status::OutOfRange);

    py::class_<florid::recording::CameraConfig>(m, "CameraConfig")
        .def(py::init<>())
        .def_readwrite("slot", &florid::recording::CameraConfig::slot)
        .def_readwrite("device", &florid::recording::CameraConfig::device)
        .def_readwrite("width", &florid::recording::CameraConfig::width)
        .def_readwrite("height", &florid::recording::CameraConfig::height)
        .def_readwrite("fps", &florid::recording::CameraConfig::fps)
        .def_readwrite("hardware_trigger", &florid::recording::CameraConfig::hardware_trigger);

    py::class_<florid::recording::InterpolatedState>(m, "InterpolatedState")
        .def_readonly("timestamp_mcu_us", &florid::recording::InterpolatedState::timestamp_mcu_us)
        .def_readonly("sample_before_us", &florid::recording::InterpolatedState::sample_before_us)
        .def_readonly("sample_after_us", &florid::recording::InterpolatedState::sample_after_us)
        .def_readonly("alpha", &florid::recording::InterpolatedState::alpha)
        .def_readonly("status", &florid::recording::InterpolatedState::status)
        .def_property_readonly("q", [](const florid::recording::InterpolatedState& state) {
            py::array_t<float> result(state.q.size());
            std::memcpy(result.mutable_data(), state.q.data(), sizeof(float) * state.q.size());
            return result;
        })
        .def_property_readonly("dq", [](const florid::recording::InterpolatedState& state) {
            py::array_t<float> result(state.dq.size());
            std::memcpy(result.mutable_data(), state.dq.data(), sizeof(float) * state.dq.size());
            return result;
        })
        .def_property_readonly("gripper_q", [](const florid::recording::InterpolatedState& state) {
            py::array_t<float> result(state.gripper_q.size());
            std::memcpy(result.mutable_data(), state.gripper_q.data(), sizeof(float) * state.gripper_q.size());
            return result;
        });

    py::class_<florid::recording::TriggerEvent>(m, "TriggerEvent")
        .def_readonly("seq_id", &florid::recording::TriggerEvent::seq_id)
        .def_readonly("timestamp_mcu_us", &florid::recording::TriggerEvent::timestamp_mcu_us)
        .def_readonly("receive_host_us", &florid::recording::TriggerEvent::receive_host_us);

    py::class_<florid::recording::AlignedRecord>(m, "AlignedRecord")
        .def_readonly("camera_slot", &florid::recording::AlignedRecord::camera_slot)
        .def_readonly("trigger_seq", &florid::recording::AlignedRecord::trigger_seq)
        .def_readonly("t_trigger_mcu_us", &florid::recording::AlignedRecord::t_trigger_mcu_us)
        .def_readonly("t_trigger_host_us", &florid::recording::AlignedRecord::t_trigger_host_us)
        .def_readonly("t_frame_host_us", &florid::recording::AlignedRecord::t_frame_host_us)
        .def_readonly("frame_index", &florid::recording::AlignedRecord::frame_index)
        .def_readonly("image_width", &florid::recording::AlignedRecord::image_width)
        .def_readonly("image_height", &florid::recording::AlignedRecord::image_height)
        .def_readonly("has_state", &florid::recording::AlignedRecord::has_state)
        .def_readonly("state", &florid::recording::AlignedRecord::state)
        .def_property_readonly("image_bgr", [](const florid::recording::AlignedRecord& record) {
            py::array_t<std::uint8_t> result(std::vector<py::ssize_t>{
                static_cast<py::ssize_t>(record.image_height),
                static_cast<py::ssize_t>(record.image_width), 3});
            if (!record.image_bgr.empty()) {
                std::memcpy(result.mutable_data(), record.image_bgr.data(), record.image_bgr.size());
            }
            return result;
        });

    py::class_<florid::recording::OpenCvRecorder>(m, "OpenCvRecorder")
        .def(py::init<florid::Arm&, std::size_t>(), py::arg("arm"), py::arg("queue_capacity") = 64)
        .def("start", &florid::recording::OpenCvRecorder::start)
        .def("stop", &florid::recording::OpenCvRecorder::stop)
        .def("running", &florid::recording::OpenCvRecorder::running)
        .def("read_once", &florid::recording::OpenCvRecorder::readOnce)
        .def("dropped_records", &florid::recording::OpenCvRecorder::droppedRecords);
#endif
}
