#ifndef FLORID_ARM_CONTROL_HPP
#define FLORID_ARM_CONTROL_HPP

#include "florid/Duration.hpp"

namespace florid {

class ArmImpl;

class ArmControl {
public:
    Duration firmwarePeriod() const;
    Duration stateAge() const;
    Duration estimatedLatency() const;
    double receiveJitterUs() const;
    double receiveHz() const;
    bool isReconnecting() const;
    void finishMotion();
    void stopControl();

private:
    friend class ArmImpl;
    ArmImpl* m_impl{nullptr};
};

} // namespace florid

#endif // FLORID_ARM_CONTROL_HPP
