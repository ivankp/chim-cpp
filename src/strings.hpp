#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <charconv>

namespace strings {

template <typename T>
struct ViewADL { };

// -----------------------------------------------------------------------------

template <typename T> requires (
    std::is_integral_v<T> ||
    std::is_same_v<double, std::remove_cv_t<T>> ||
    std::is_same_v<float, std::remove_cv_t<T>>
)
class Number {
    unsigned char n;
    char str[
        std::is_integral_v<T>
        ? ((616 * sizeof(T)) >> 8) + 1 + std::is_signed_v<T>
        : std::is_same_v<T, double> ? 24 : 16
    ];

public:
    constexpr Number(T x) noexcept
        : n(std::to_chars(str, str + sizeof(str), x).ptr - str)
    { }
    explicit constexpr operator std::string_view() const noexcept {
        return { str, n };
    }
};

// -----------------------------------------------------------------------------
namespace detail {

template <typename T>
[[nodiscard]]
inline constexpr decltype(auto) view(const T& x) noexcept {
    if constexpr (requires { ViewADL<T>::view(x); }) {
        return ViewADL<T>::view(x);
    } else if constexpr (std::is_same_v<T, bool>) {
        return std::string_view(x ? "true" : "false");
    } else if constexpr (std::is_same_v<T, char>) {
        return std::string_view(&x, 1);
    } else if constexpr (std::is_arithmetic_v<T>) {
        return Number<T>(x);
    } else {
        return x;
    }
}

}
// -----------------------------------------------------------------------------

[[nodiscard]]
inline constexpr std::string cat() noexcept {
    return { };
}

[[nodiscard]]
inline constexpr std::string cat(std::string x) noexcept {
    return x;
}

[[nodiscard]]
inline constexpr std::string cat(const auto& x) noexcept {
    return std::string(std::string_view(detail::view(x)));
}

// -----------------------------------------------------------------------------

template <typename... T>
[[nodiscard]]
inline constexpr std::string cat(T... x) noexcept
requires (sizeof...(x) >= 2 && (std::is_same_v<T, std::string_view> && ...))
{
    std::string s((x.size() + ...), {});
    char* buf = s.data();
    ((std::memcpy(buf, x.data(), x.size()), buf += x.size()), ...);
    return s;
}

template <typename... T>
[[nodiscard]]
inline constexpr std::string cat(T&&... x) noexcept
requires (sizeof...(x) >= 2 && !(std::is_same_v<T, std::string_view> && ...))
{
    return cat(std::string_view(detail::view(x))...);
}

} // namespace strings
