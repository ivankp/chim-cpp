#pragma once

#include <filesystem>

struct Mmapped {
    const std::filesystem::path path;
    int fd;
    char* m;
    size_t size;

    Mmapped(const std::filesystem::path&);
    ~Mmapped();
};
