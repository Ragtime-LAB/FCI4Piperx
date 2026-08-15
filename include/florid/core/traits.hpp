#ifndef FLORID_CORE_TRAITS_HPP
#define FLORID_CORE_TRAITS_HPP

#include <type_traits>

namespace florid::detail {

template <bool B>
using bool_constant = std::integral_constant<bool, B>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <typename T>
using remove_reference = std::remove_reference_t<T>;

template <typename T>
using remove_cv = std::remove_cv_t<T>;

template <typename T>
using decay_simple = std::decay_t<T>;

} // namespace florid::detail

namespace florid {

struct JointMIT;

template <typename T>
struct is_control_command : detail::false_type {};

template <> struct is_control_command<JointMIT> : detail::true_type {};

} // namespace florid

#endif // FLORID_CORE_TRAITS_HPP
