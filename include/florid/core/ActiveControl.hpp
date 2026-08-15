#ifndef FLORID_CORE_ACTIVE_CONTROL_HPP
#define FLORID_CORE_ACTIVE_CONTROL_HPP

#include "florid/ArmState.hpp"
#include "florid/ControlTypes.hpp"

#include <functional>
#include <utility>

namespace florid {

template <typename ControlType>
class ActiveControl {
public:
    using ReadFunc = std::function<ArmState()>;
    using WriteFunc = std::function<void(const ControlType&)>;

    ActiveControl(ReadFunc s_read, WriteFunc s_write)
        : m_read(std::move(s_read)), m_write(std::move(s_write)) {}

    ~ActiveControl() = default;

    ActiveControl(const ActiveControl&) = delete;
    ActiveControl& operator=(const ActiveControl&) = delete;
    ActiveControl(ActiveControl&&) noexcept = default;
    ActiveControl& operator=(ActiveControl&&) noexcept = default;

    ArmState readOnce() { return m_read(); }
    void writeOnce(const ControlType& s_cmd) { m_write(s_cmd); }

private:
    ReadFunc m_read;
    WriteFunc m_write;
};

} // namespace florid

#endif // FLORID_CORE_ACTIVE_CONTROL_HPP
