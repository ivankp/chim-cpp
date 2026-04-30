#pragma once

#include <cstddef>

class Mmapped {
    const char* path;
    int fd;
    char* m;
    size_t size;

public:
    Mmapped(const char* path);
    ~Mmapped();
};
