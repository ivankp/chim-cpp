#pragma once

#include <filesystem>

class Mmapped {
    const std::filesystem::path path;
    int fd;
    char* m;
    size_t size;

public:
    Mmapped(const std::filesystem::path&);
    ~Mmapped();
};
