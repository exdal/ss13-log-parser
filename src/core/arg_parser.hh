#pragma once

#include "core/hash.hh"

constexpr std::string escape_str(std::string_view str) {
    std::string r;
    r.reserve(str.size());

    for (c8 c : str) {
        switch (c) {
                // clang-format off
            case '\'': r += "\\\'"; break;
            case '\"': r += "\\\""; break;
            case '\?': r += "\\?";  break;
            case '\\': r += "\\\\"; break;
            case '\a': r += "\\a";  break;
            case '\b': r += "\\b";  break;
            case '\f': r += "\\f";  break;
            case '\n': r += "\\n";  break;
            case '\r': r += "\\r";  break;
            case '\t': r += "\\t";  break;
            case '\v': r += "\\v";  break;
            default: r += c; break;
                // clang-format on
        }
    }

    return r;
}

struct ArgParser {
private:
    struct Arg {
        u64 param_hash = 0;
        std::string value = {};
    };
    std::vector<Arg> args = {};

public:
    ArgParser(ls::span<c8 *> args_) {
        for (usize i = 0; i < args_.size(); i++) {
            std::string_view param_sv(args_[i]);
            u64 h = fnv64_str(param_sv);

            if (param_sv.starts_with("--") && i + 1 < args_.size()) {
                std::string_view value_sv(args_[i + 1]);
                auto value_str = escape_str(value_sv);
                if (!value_str.starts_with("--")) {
                    args.emplace_back(h, value_str);
                    i++;
                }
            } else {
                // It is not param, count it as a value
                args.emplace_back(h, escape_str(param_sv));
            }
        }
    }

    constexpr std::optional<std::string_view> operator[](std::string_view arg) {
        u64 hash = fnv64_str(arg);
        for (auto &[h, v] : args) {
            if (h == hash) {
                return v;
            }
        }

        return std::nullopt;
    }

    constexpr std::optional<std::string_view> operator[](usize i) {
        if (i >= args.size()) {
            return std::nullopt;
        }

        return args[i].value;
    }
};
