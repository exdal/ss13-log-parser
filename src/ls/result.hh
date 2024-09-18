#pragma once

template<typename ErrorT>
concept ErrorConcept = requires(ErrorT v) { ErrorT::Success; };

template<typename T, ErrorConcept Error>
struct Result {
    using SelfT = Result<T, Error>;

    static_assert(std::is_default_constructible_v<T>);
    static_assert(!std::is_same_v<Error, T>);

private:
    Error err;
    T value;

public:
    Result(Error error)
        : err(error) {}
    Result(T &&value)
        : err(Error::Success),
          value(std::forward<T>(value)) {}
    explicit Result(T &&value, Error error)
        : err(error),
          value(std::forward<T>(value)) {}

    SelfT &operator=(SelfT &other) = delete;
    SelfT &operator=(SelfT &&other) noexcept {
        err = other.err;
        value = std::move(other.value);
        return *this;
    }

    [[nodiscard]] Error error() const noexcept { return err; }
    [[nodiscard]] T &get() noexcept { return value; }
    [[nodiscard]] T *get_if() noexcept {
        if (err != Error::Success)
            return nullptr;
        return std::addressof(value);
    }
    [[nodiscard]] T &get_unsafe() noexcept { return value; }
    [[nodiscard]] T *operator->() noexcept { return std::addressof(value); }
    [[nodiscard]] const T *operator->() const noexcept { return std::addressof(value); }
    [[nodiscard]] T &&operator*() && noexcept { return std::move(value); }
    [[nodiscard]] operator bool() const noexcept { return err == Error::Success; }
};
