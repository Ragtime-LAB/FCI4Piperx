#include "florid/recording/Timeline.hpp"

#include "RPL/Deserializer.hpp"
#include "RPL/Parser.hpp"
#include "RPL/Serializer.hpp"
#include "RPL/Packets/USBAck.hpp"
#include "RPL/USBTransport.hpp"
#include "fci_protocol/arm/packets.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <vector>

using florid::recording::AngleTimeline;
using florid::recording::TimedArmSample;

void test_interpolation() {
    AngleTimeline timeline(8, 5000);
    TimedArmSample first;
    first.timestamp_mcu_us = 1000;
    first.seq = 1;
    first.q[0] = 1.0f;
    first.dq[0] = 2.0f;
    timeline.push(first);

    TimedArmSample second = first;
    second.timestamp_mcu_us = 3000;
    second.seq = 2;
    second.q[0] = 5.0f;
    second.dq[0] = 4.0f;
    timeline.push(second);

    auto result = timeline.interpolate(2000);
    assert(result.has_value());
    assert(result->status == decltype(result->status)::Interpolated);
    assert(result->alpha == 0.5f);
    assert(result->q[0] == 3.0f);
    assert(result->dq[0] == 3.0f);
    assert(result->sample_before_seq == 1);
    assert(result->sample_after_seq == 2);
    assert(!timeline.interpolate(999).has_value());

    TimedArmSample third = second;
    third.timestamp_mcu_us = 12000;
    third.q[0] = 20.0f;
    timeline.push(third);
    auto gap = timeline.interpolate(7500);
    assert(gap.has_value());
    assert(gap->status == decltype(gap->status)::Gap);
}

struct DummyTick {
    using tick_type = std::uint32_t;
    static tick_type now() { return 1; }
};

void test_trigger_events_are_not_collapsed() {
    using Session = RPL::USBTransport<
        RPL::AckManager<DummyTick>,
        void (*)(const std::uint8_t*, std::size_t),
        USBAck,
        fci::arm::TriggerPacket>;

    RPL::Serializer<USBAck, fci::arm::TriggerPacket> serializer;
    fci::arm::TriggerPacket first{100, 1};
    fci::arm::TriggerPacket second{200, 2};
    std::vector<std::uint8_t> bytes(
        RPL::Serializer<USBAck, fci::arm::TriggerPacket>::frame_size<fci::arm::TriggerPacket>() * 2);
    auto serialized = serializer.serialize(bytes.data(), bytes.size(), first, second);
    assert(serialized.has_value());

    Session session;
    std::vector<fci::arm::TriggerPacket> received;
    session.on_packet([&](std::uint16_t cmd, std::span<const std::uint8_t> p1,
                          std::span<const std::uint8_t> p2) {
        if (cmd != fci::arm::to_u16(fci::arm::Command::Trigger)) return;
        fci::arm::TriggerPacket packet{};
        std::memcpy(&packet, p1.data(), p1.size());
        if (!p2.empty()) std::memcpy(reinterpret_cast<std::uint8_t*>(&packet) + p1.size(), p2.data(), p2.size());
        received.push_back(packet);
    });
    auto parsed = session.receive(bytes.data(), *serialized);
    assert(parsed.has_value());
    assert(received.size() == 2);
    assert(received[0].timestamp_us == 100);
    assert(received[1].seq_id == 2);
}

int main() {
    test_interpolation();
    test_trigger_events_are_not_collapsed();
    std::puts("recording tests passed");
    return 0;
}
