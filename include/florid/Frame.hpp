#ifndef FLORID_FRAME_HPP
#define FLORID_FRAME_HPP

#include <cstdint>

namespace florid
{
    enum class Frame : uint8_t
    {
        kJoint1 = 1,
        kJoint2,
        kJoint3,
        kJoint4,
        kJoint5,
        kJoint6,
        kFlange = 6,
        kEndEffector = 6,
    };

} // namespace florid

#endif // FLORID_FRAME_HPP
