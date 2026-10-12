#pragma once

#include <cstddef>

struct File {
    int fd = -1;

    File(const char* path);
    ~File();
};

struct RegularFile: File {
    size_t size;

    RegularFile(const char* path);
};

struct WholeFile {
    char* m = nullptr;
    size_t size = 0;

    WholeFile(const char* path);
    ~WholeFile();

    WholeFile(const WholeFile&) = delete;
    WholeFile& operator=(const WholeFile&) = delete;

    WholeFile(WholeFile&& other) noexcept : m(other.m), size(other.size) {
        other.m = nullptr;
        other.size = 0;
    }
    WholeFile& operator=(WholeFile&& other) noexcept {
        m = other.m;
        other.m = nullptr;
        size = other.size;
        other.size = 0;
        return *this;
    }
};

struct MappedFile {
    char* m = nullptr;
    size_t size = 0;

    MappedFile(const char* path);
    ~MappedFile();

    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;

    MappedFile(MappedFile&& other) noexcept : m(other.m), size(other.size) {
        other.m = nullptr;
        other.size = 0;
    }
    MappedFile& operator=(MappedFile&& other) noexcept {
        m = other.m;
        other.m = nullptr;
        size = other.size;
        other.size = 0;
        return *this;
    }
};

bool Exists(const char* path) noexcept;
