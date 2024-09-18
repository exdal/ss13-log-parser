#pragma once

struct ThreadStack {
    u8 *ptr = nullptr;

    ThreadStack();
    ~ThreadStack();
};

inline ThreadStack &get_thread_stack() {
    thread_local ThreadStack stack;
    return stack;
}

struct ScopedStack {
    u8 *ptr = nullptr;

    ScopedStack();
    ScopedStack(const ScopedStack &) = delete;
    ScopedStack(ScopedStack &&) = delete;
    ~ScopedStack();

    auto operator=(const ScopedStack &) = delete;
    auto operator=(ScopedStack &&) = delete;

    template<typename T>
    T *alloc() {
        auto &stack = get_thread_stack();
        T *v = reinterpret_cast<T *>(stack.ptr);
        stack.ptr = ls::align_up(stack.ptr + sizeof(T), alignof(T));

        return v;
    }

    template<typename T>
    ls::span<T> alloc(usize count) {
        auto &stack = get_thread_stack();
        T *v = reinterpret_cast<T *>(stack.ptr);
        stack.ptr = ls::align_up(stack.ptr + sizeof(T) * count, alignof(T));

        return { v, count };
    }

    template<typename T, typename... ArgsT>
    ls::span<T> alloc_n(ArgsT &&...args) {
        usize count = sizeof...(ArgsT);
        ls::span<T> spn = alloc<T>(count);
        std::construct_at(reinterpret_cast<T *>(spn.data()), std::forward<ArgsT>(args)...);

        return spn;
    }

    std::string_view alloc_sv(usize size) {
        auto &stack = get_thread_stack();
        const c8 *v = reinterpret_cast<const c8 *>(stack.ptr);
        stack.ptr = ls::align_up(stack.ptr + sizeof(c8) * size, alignof(c8));

        return { v, size };
    }

    template<typename... ArgsT>
    std::string_view format(const fmt::format_string<ArgsT...> fmt, ArgsT &&...args) {
        auto &stack = get_thread_stack();
        c8 *begin = reinterpret_cast<c8 *>(stack.ptr);
        c8 *end = fmt::vformat_to(begin, fmt.get(), fmt::make_format_args(args...));
        *end = '\0';
        stack.ptr = ls::align_up(reinterpret_cast<u8 *>(end + 1), 8);

        return { begin, end };
    }

    std::string_view to_upper(std::string_view str) {
        auto &stack = get_thread_stack();
        c8 *begin = reinterpret_cast<c8 *>(stack.ptr);
        memcpy(begin, str.data(), str.length());
        c8 *end = reinterpret_cast<c8 *>(stack.ptr + str.length());
        stack.ptr = ls::align_up(reinterpret_cast<u8 *>(end + 1), 8);

        std::transform(begin, end, begin, ::toupper);
        *end = '\0';

        return { begin, end };
    }

    std::string_view to_lower(std::string_view str) {
        auto &stack = get_thread_stack();
        c8 *begin = reinterpret_cast<c8 *>(stack.ptr);
        memcpy(begin, str.data(), str.length());
        c8 *end = reinterpret_cast<c8 *>(stack.ptr + str.length());
        stack.ptr = ls::align_up(reinterpret_cast<u8 *>(end + 1), 8);

        std::transform(begin, end, begin, ::tolower);
        *end = '\0';

        return { begin, end };
    }
};
