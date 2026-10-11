#pragma once

#include "strings.hh"

#include <exception>

#include <errno.h>

class Error : public std::exception {
    std::string message;

public:
    struct code_t { } static code;

    void operator()(code_t, const auto&... args) noexcept {
        const auto e = errno;
        if constexpr (sizeof...(args) > 0) {
            message = strings::cat(
                message, "\n[" + message.empty(),
                e, "]: ", args..., ": ", ::strerror(e)
            );
        } else {
            message = strings::cat(
                message, "\n[" + message.empty(),
                e, "]: ", ::strerror(e)
            );
        }
    };

    void operator()(const auto&... args) noexcept {
        message = strings::cat(message, "\n" + message.empty(), args...);
    };

    const char* what() const noexcept override {
        return message.c_str();
    }
};
