#pragma once

namespace ls {
template<typename T>
constexpr static T align_up(T size, u64 alignment) {
    return T((u64(size) + (alignment - 1)) & ~(alignment - 1));
}

template<typename T>
constexpr static T align_down(T size, u64 alignment) {
    return T(u64(size) & ~(alignment - 1));
}

template<typename T>
constexpr static T kib_to_bytes(const T x) {
    return x << static_cast<T>(10);
}

template<typename T>
constexpr static T mib_to_bytes(const T x) {
    return x << static_cast<T>(20);
}

}  // namespace ls
