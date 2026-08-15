#include "florid/detail/AstrialUSBTransport.hpp"

#include "astrial/Serial.hpp"
#include "astrial/SerialBuilder.hpp"
#include "astrial/Types.hpp"

namespace florid {

AstrialUSBTransport::AstrialUSBTransport(const std::string &s_port_path,
                                         std::uint32_t s_baud_rate) {
  auto s_result = Serial::builder()
                      .baud_rate(s_baud_rate)
                      .parity(Parity::None)
                      .stop_bits(StopBits::One)
                      .open(s_port_path);

  if (!s_result) {
    throw std::runtime_error("Failed to open USB device: " + s_port_path +
                             " (" + s_result.error().message() + ")");
  }

  m_serial = std::make_unique<Serial>(std::move(s_result.value()));
}

AstrialUSBTransport::~AstrialUSBTransport() {
  if (m_serial) {
    m_serial->close();
  }
}

AstrialUSBTransport::AstrialUSBTransport(AstrialUSBTransport &&s_other) noexcept
    : m_serial(std::move(s_other.m_serial)),
      m_recv_callback(s_other.m_recv_callback),
      m_recv_context(s_other.m_recv_context) {
  s_other.m_recv_callback = nullptr;
  s_other.m_recv_context = nullptr;
}

AstrialUSBTransport &
AstrialUSBTransport::operator=(AstrialUSBTransport &&s_other) noexcept {
  if (this != &s_other) {
    m_serial = std::move(s_other.m_serial);
    m_recv_callback = s_other.m_recv_callback;
    m_recv_context = s_other.m_recv_context;
    s_other.m_recv_callback = nullptr;
    s_other.m_recv_context = nullptr;
  }
  return *this;
}

bool AstrialUSBTransport::send(const std::uint8_t *s_data, std::size_t s_size) {
  if (!m_serial)
    return false;
  std::lock_guard<std::mutex> s_lock(m_write_mutex);
  auto s_r = m_serial->write(std::span<const std::uint8_t>(s_data, s_size));
  return s_r.has_value();
}

void AstrialUSBTransport::setReceiveCallback(ReceiveFunctor s_callback,
                                             void *s_context) {
  m_recv_callback = s_callback;
  m_recv_context = s_context;

  if (m_recv_callback && m_serial) {
    m_serial->on_data([this](std::span<const std::uint8_t> s_data) {
      if (m_recv_callback) {
        m_recv_callback(m_recv_context, s_data.data(), s_data.size());
      }
    });
  }
}

void AstrialUSBTransport::poll() {
  // Astrial has its own internal ASIO io_context thread driving async I/O.
  // No polling needed on the host platform.
}

bool AstrialUSBTransport::isConnected() const { return m_serial != nullptr; }

std::vector<UsbDeviceInfo> AstrialUSBTransport::listDevices() {
  std::vector<UsbDeviceInfo> s_devices;
  auto s_ports = Serial::list_ports();
  for (const auto &s_port : s_ports) {
    UsbDeviceInfo s_info;
    s_info.m_port_name = s_port.port_name;
    s_info.m_description = s_port.description;
    s_info.m_vendor_id = s_port.vendor_id;
    s_info.m_product_id = s_port.product_id;
    s_info.m_serial_number = s_port.serial_number;
    s_info.m_manufacturer = s_port.manufacturer;
    s_devices.push_back(std::move(s_info));
  }
  return s_devices;
}

} // namespace florid
