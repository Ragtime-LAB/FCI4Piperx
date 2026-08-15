#ifndef FCI_PROTOCOL_VERSION_HPP
#define FCI_PROTOCOL_VERSION_HPP

#include <cstdint>

namespace fci {

// Semantic version, composed of three uint8_t fields.
struct Semver {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t patch;
};

inline constexpr Semver MakeSemver(std::uint8_t major, std::uint8_t minor,
                                   std::uint8_t patch) {
    return Semver{major, minor, patch};
}

} // namespace fci

#endif // FCI_PROTOCOL_VERSION_HPP
