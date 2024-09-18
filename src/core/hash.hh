#pragma once

namespace detail {
constexpr u64 fnv64_val = 14695981039346656037_u64;
constexpr u64 fnv64_prime = 1099511628211_u64;
}  // namespace detail

constexpr u64 fnv64(const c8 *data, usize data_size) {
    u64 fnv = detail::fnv64_val;

    for (u32 i = 0; i < data_size; i++)
        fnv = (fnv * detail::fnv64_prime) ^ data[i];

    return fnv;
}

constexpr u64 fnv64_str(std::string_view str) {
    return fnv64(str.data(), str.length());
}

consteval u64 fnv64_c(std::string_view str) {
    return fnv64(str.data(), str.length());
}
