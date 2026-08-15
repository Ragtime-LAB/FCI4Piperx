#ifndef FLORID_DETAIL_TICK_PROVIDER_HPP
#define FLORID_DETAIL_TICK_PROVIDER_HPP

#include <cstdint>

namespace florid::detail {

struct MonotonicTickProvider {
    using tick_type = std::uint32_t;

    static tick_type now();
};

} // namespace florid::detail

#endif // FLORID_DETAIL_TICK_PROVIDER_HPP
