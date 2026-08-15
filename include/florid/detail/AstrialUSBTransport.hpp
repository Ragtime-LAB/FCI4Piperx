#ifndef FLORID_DETAIL_ASTRIAL_USB_TRANSPORT_HPP
#define FLORID_DETAIL_ASTRIAL_USB_TRANSPORT_HPP

#include "florid/detail/Transport.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class Serial;

namespace florid {

struct UsbDeviceInfo {
  std::string m_port_name;
  std::string m_description;
  std::uint16_t m_vendor_id;
  std::uint16_t m_product_id;
  std::string m_serial_number;
  std::string m_manufacturer;
};

class AstrialUSBTransport : public Transport {
public:
  explicit AstrialUSBTransport(const std::string &s_port_path,
                               std::uint32_t s_baud_rate = 115200);
  ~AstrialUSBTransport() override;

  AstrialUSBTransport(const AstrialUSBTransport &) = delete;
  AstrialUSBTransport &operator=(const AstrialUSBTransport &) = delete;
  AstrialUSBTransport(AstrialUSBTransport &&) noexcept;
  AstrialUSBTransport &operator=(AstrialUSBTransport &&) noexcept;

  bool send(const std::uint8_t *s_data, std::size_t s_size) override;

  void setReceiveCallback(ReceiveFunctor s_callback, void *s_context) override;

  void poll() override;

  bool isConnected() const;

  static std::vector<UsbDeviceInfo> listDevices();

private:
  std::unique_ptr<Serial> m_serial;
  ReceiveFunctor m_recv_callback{nullptr};
  void *m_recv_context{nullptr};
  std::mutex m_write_mutex;
  // TODO: remove this mutex for performance
};

} // namespace florid

#endif // FLORID_DETAIL_ASTRIAL_USB_TRANSPORT_HPP
