#include <florid/detail/tick.hpp>
#include <chrono>

namespace florid::detail
{
    double get_tick_ms()
    {
        const auto tp = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(tp.time_since_epoch()).count();
    }
}
