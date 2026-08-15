#include "florid/detail/TickProvider.hpp"

#include <chrono>

namespace florid::detail {

MonotonicTickProvider::tick_type MonotonicTickProvider::now() {
    static const auto s_start = std::chrono::steady_clock::now();
    auto s_now = std::chrono::steady_clock::now();
    return static_cast<tick_type>(
        std::chrono::duration_cast<std::chrono::milliseconds>(s_now - s_start).count());
}

} // namespace florid::detail
