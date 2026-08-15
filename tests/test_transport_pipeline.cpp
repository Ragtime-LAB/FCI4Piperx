#include "florid/detail/ArmImpl.hpp"
#include "florid/detail/Seqlock.hpp"

#include "fci_protocol/arm/packets.hpp"
#include "fci_protocol/session/arm_control_session.hpp"
#include "fci_protocol/transport/byte_stream_transport.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

using namespace florid;

// ────────────────────────────────────────────────────────────
//  MockTransport: captures sent bytes, can inject received bytes
// ────────────────────────────────────────────────────────────

class MockTransport : public Transport {
public:
    bool send(const std::uint8_t* s_data, std::size_t s_size) override {
        m_sent.insert(m_sent.end(), s_data, s_data + s_size);

        // Auto-respond to GetDeviceInfoRequest so ArmImpl construction succeeds
        if (s_size >= 7 && s_data[0] == 0xA5) {
            std::uint16_t s_cmd = static_cast<std::uint16_t>(s_data[3])
                               | (static_cast<std::uint16_t>(s_data[4]) << 8);
            if (s_cmd == 0x6215) {
                std::uint8_t s_req_id = s_data[5];
                inject(s_makeAckFrame(s_req_id));
                inject(s_makeDeviceInfoFrame(s_req_id));
            }
        }
        return true;
    }

    void setReceiveCallback(ReceiveFunctor s_callback, void* s_context) override {
        m_recv_cb = s_callback;
        m_recv_ctx = s_context;
    }

    void poll() override {}

    // Inject bytes into ArmImpl's receive pipeline
    void inject(const std::vector<uint8_t>& s_bytes) {
        if (m_recv_cb) {
            m_recv_cb(m_recv_ctx, s_bytes.data(), s_bytes.size());
        }
    }

    std::vector<uint8_t> m_sent;
    ReceiveFunctor m_recv_cb{nullptr};
    void* m_recv_ctx{nullptr};

private:
    static std::vector<uint8_t> s_makeAckFrame(std::uint8_t s_req_id) {
        // cmd = USBAck (0x6FF0), little-endian -> F0 6F
        return {
            0xA5, 0x02, 0x00,             // start + length=2
            0xF0, 0x6F,                   // cmd = USBAck (0x6FF0)
            s_req_id, 0x00                // status = Ok
        };
    }

    static std::vector<uint8_t> s_makeDeviceInfoFrame(std::uint8_t s_req_id) {
        // Frame: 5-byte header + 72-byte payload = 77 bytes
        // Payload: req_id(1) + protocol_version(3) + fw_version(3)
        //          + board_name(32) + custom_name(32) + fw_type(1)
        std::vector<uint8_t> s_frame(77, 0);
        s_frame[0] = 0xA5;
        s_frame[1] = 0x48;  s_frame[2] = 0x00;     // length = 72
        s_frame[3] = 0x16;  s_frame[4] = 0x62;     // cmd = 0x6216 (GetDeviceInfoResponse)
        s_frame[5] = s_req_id;
        s_frame[6] = 1;                             // protocol_version.major
        s_frame[9] = 2;                             // fw_version.major
        s_frame[10] = 3;                            // fw_version.minor
        s_frame[11] = 1;                            // fw_version.patch
        return s_frame;
    }
};

// ────────────────────────────────────────────────────────────
//  Helper: serialize a known-good ArmStatus frame
// ────────────────────────────────────────────────────────────

struct DummyTick {
    using tick_type = std::uint32_t;
    static tick_type now() { return 1; }
};

std::vector<std::uint8_t> serializeArmStatus(fci::arm::ArmStatus& s_status) {
    using Session = RPL::USBTransport<
        RPL::AckManager<DummyTick>,
        std::function<void(const std::uint8_t*, std::size_t)>,
        USBAck,
        fci::arm::ArmStatus>;

    std::vector<uint8_t> s_bytes;
    Session s_sess;
    s_sess.on_send([&s_bytes](const uint8_t* d, size_t n) {
        s_bytes.assign(d, d + n);
    });

    auto s_result = s_sess.notify(s_status);
    assert(s_result.has_value());
    return s_bytes;
}

// ────────────────────────────────────────────────────────────
//  Tests
// ────────────────────────────────────────────────────────────

void test_arm_status_roundtrip() {
    printf("Test 1: ArmStatus round-trip through MockTransport\n");

    auto s_transport = std::make_unique<MockTransport>();
    auto* s_mock = static_cast<MockTransport*>(s_transport.get());

    // Create ArmImpl — MockTransport auto-responds to GetDeviceInfoRequest
    ArmImpl s_impl(std::move(s_transport));

    // Verify device info was fetched from mock response
    assert(s_impl.firmwarePeriodUs() == 2000);
    assert(s_impl.getDeviceInfo().protocol_version.major == 1);
    assert(s_impl.getDeviceInfo().fw_version.major == 2);
    assert(s_impl.getDeviceInfo().fw_version.minor == 3);
    assert(s_impl.getDeviceInfo().fw_version.patch == 1);

    // Build a fake ArmStatus (dual-arm: 12 joints, 2 grippers)
    fci::arm::ArmStatus s_status{};
    s_status.seq = 123;
    s_status.timestamp_us = 999000;
    s_status.errors = 0x0A;
    s_status.status.q[0] = 1.0f;
    s_status.status.q[1] = 2.0f;
    s_status.status.q[2] = 3.0f;
    s_status.status.q[3] = 4.0f;
    s_status.status.q[4] = 5.0f;
    s_status.status.q[5] = 6.0f;
    s_status.status.q[6] = 7.0f;
    s_status.status.q[11] = 12.0f;
    s_status.status.dq[0] = 0.1f;
    s_status.base_gravity[0] = 0.0f;
    s_status.base_gravity[1] = 0.0f;
    s_status.base_gravity[2] = -9.81f;
    s_status.base_gravity[3] = 0.0f;
    s_status.base_gravity[4] = 0.0f;
    s_status.base_gravity[5] = -9.81f;
    s_status.O_T_EE[15] = 1.0f;
    s_status.F_ext[0] = 5.5f;
    s_status.gripper.q[0] = 0.05f;
    s_status.gripper.q[1] = 0.07f;

    // Serialize the ArmStatus to wire bytes
    auto s_frame = serializeArmStatus(s_status);
    printf("  Frame size: %zu bytes\n", s_frame.size());

    // Inject the bytes into ArmImpl via MockTransport
    s_mock->inject(s_frame);

    // Read from ArmImpl
    auto s_state = s_impl.readOnce();
    printf("  seq=%u q0=%f q1=%f q6=%f err=0x%02X gz0=%f\n",
           s_state.m_seq, s_state.m_q[0], s_state.m_q[1], s_state.m_q[6],
           s_state.m_errors, s_state.m_base_gravity[2]);

    assert(s_state.m_seq == 123);
    assert(s_state.m_q[0] == 1.0f);
    assert(s_state.m_q[2] == 3.0f);
    assert(s_state.m_q[6] == 7.0f);
    assert(s_state.m_q[11] == 12.0f);
    assert(s_state.m_dq[0] == 0.1f);
    assert(s_state.m_errors == 0x0A);
    assert(s_state.m_base_gravity[2] == -9.81f);
    assert(s_state.m_base_gravity[5] == -9.81f);
    assert(s_state.m_O_T_EE[15] == 1.0f);
    assert(s_state.m_F_ext[0] == 5.5f);
    assert(s_state.m_gripper_q[0] == 0.05f);
    assert(s_state.m_gripper_q[1] == 0.07f);

    printf("  PASS\n");
}

void test_multiple_frames() {
    printf("Test 2: Multiple sequential ArmStatus frames\n");

    auto s_transport = std::make_unique<MockTransport>();
    auto* s_mock = static_cast<MockTransport*>(s_transport.get());
    ArmImpl s_impl(std::move(s_transport));

    for (uint32_t s_i = 0; s_i < 10; ++s_i) {
        fci::arm::ArmStatus s_status{};
        s_status.seq = s_i;
        s_status.status.q[0] = static_cast<float>(s_i);

        auto s_frame = serializeArmStatus(s_status);
        s_mock->inject(s_frame);

        auto s_state = s_impl.readOnce();
        assert(s_state.m_seq == s_i);
        assert(s_state.m_q[0] == static_cast<float>(s_i));
    }

    printf("  PASS\n");
}

void test_parse_garbage() {
    printf("Test 3: Garbage data does not crash\n");

    auto s_transport = std::make_unique<MockTransport>();
    auto* s_mock = static_cast<MockTransport*>(s_transport.get());
    ArmImpl s_impl(std::move(s_transport));

    // Inject random garbage
    std::vector<uint8_t> s_garbage(256, 0xFF);
    s_mock->inject(s_garbage);

    // Follow with a valid frame
    fci::arm::ArmStatus s_status{};
    s_status.seq = 999;
    s_status.status.q[0] = 7.5f;
    auto s_frame = serializeArmStatus(s_status);
    s_mock->inject(s_frame);

    auto s_state = s_impl.readOnce();
    assert(s_state.m_seq == 999);
    assert(s_state.m_q[0] == 7.5f);

    printf("  PASS\n");
}

void test_control_loop() {
    printf("Test 4: Control loop sends commands\n");

    auto s_transport = std::make_unique<MockTransport>();
    auto* s_mock = static_cast<MockTransport*>(s_transport.get());
    ArmImpl s_impl(std::move(s_transport));

    // Prepare 5 ArmStatus frames
    for (uint32_t s_i = 0; s_i < 5; ++s_i) {
        fci::arm::ArmStatus s_status{};
        s_status.seq = s_i;
        s_status.status.q[0] = static_cast<float>(s_i);
        auto s_frame = serializeArmStatus(s_status);

        // Inject, then call readOnce in the same thread (no separate control thread)
        s_mock->inject(s_frame);
        s_mock->m_sent.clear(); // reset sent buffer between iterations
    }

    // Inject one more frame for the control loop to consume
    fci::arm::ArmStatus s_status{};
    s_status.seq = 100;
    s_status.status.q[0] = 1.0f;
    auto s_frame = serializeArmStatus(s_status);
    s_mock->inject(s_frame);

    // Run control loop with a simple torque callback
    int s_call_count = 0;
    s_impl.s_controlLoop([&](const ArmState& s_state, ArmControl&) -> JointMIT {
        s_call_count++;
        JointMIT s_cmd;
        s_cmd.m_tau[0] = s_state.m_q[0] * 10.0f;
        if (s_call_count >= 1) s_cmd.m_motion_finished = true;
        return s_cmd;
    });

    assert(s_call_count == 1);
    assert(!s_mock->m_sent.empty());

    printf("  PASS (callbacks=%d, sent_bytes=%zu)\n", s_call_count, s_mock->m_sent.size());
}

int main() {
    test_arm_status_roundtrip();
    test_multiple_frames();
    test_parse_garbage();
    test_control_loop();
    printf("\nAll tests passed.\n");
    return 0;
}
