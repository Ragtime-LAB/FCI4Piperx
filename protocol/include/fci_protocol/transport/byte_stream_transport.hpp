#ifndef FCI_PROTOCOL_TRANSPORT_BYTE_STREAM_TRANSPORT_HPP
#define FCI_PROTOCOL_TRANSPORT_BYTE_STREAM_TRANSPORT_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace fci::transport {

template <typename WriteCallback>
class ByteStreamTransport {
public:
    ByteStreamTransport() = default;

    explicit ByteStreamTransport(WriteCallback write_callback)
        : write_callback_(std::move(write_callback))
    {
    }

    void set_write_callback(WriteCallback write_callback)
    {
        write_callback_ = std::move(write_callback);
    }

    void operator()(const std::uint8_t* data, std::size_t len) const
    {
        if constexpr (std::is_pointer_v<WriteCallback>) {
            if (write_callback_ != nullptr) {
                write_callback_(data, len);
            }
        } else {
            write_callback_(data, len);
        }
    }

private:
    WriteCallback write_callback_ {};
};

using FunctionByteStreamTransport =
    ByteStreamTransport<std::function<void(const std::uint8_t*, std::size_t)>>;

using FunctionPointerByteStreamTransport =
    ByteStreamTransport<void (*)(const std::uint8_t*, std::size_t)>;

} // namespace fci::transport

#endif // FCI_PROTOCOL_TRANSPORT_BYTE_STREAM_TRANSPORT_HPP
