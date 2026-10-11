#pragma once

#include <cstddef>

struct File {
  int fd;

  File(const char* name);
  ~File();
};

struct RegularFile: File {
  size_t size;

  RegularFile(const char* name);
};

struct WholeFile {
  char* m;
  size_t size;

  WholeFile(const char* name);
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

bool Exists(const char* name) noexcept;
