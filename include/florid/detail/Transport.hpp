#ifndef FLORID_TRANSPORT_HPP
#define FLORID_TRANSPORT_HPP

#include <cstdint>
#include <cstddef>

namespace florid {

class Transport {
public:
    using ReceiveFunctor = void (*)(void* s_context, const std::uint8_t* s_data, std::size_t s_size);

    virtual ~Transport() = default;

    virtual bool send(const std::uint8_t* s_data, std::size_t s_size) = 0;

    virtual void setReceiveCallback(ReceiveFunctor s_callback, void* s_context) = 0;

    virtual void poll() = 0;
};

} // namespace florid

#endif // FLORID_TRANSPORT_HPP
