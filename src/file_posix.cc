#include "file_posix.hh"

#include "strings.hh"

#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <cstdlib>

#define STR1(X) #X
#define STR(X) STR1(X)

#ifdef PCALL
#error "PCALL macro already defined"
#endif
#define PCALL(F) [](auto&&... x) { \
    const auto r = ::F(x...); \
    if (r == -1) throw std::runtime_error(cat( \
        STR(F) "(): ", std::strerror(errno) \
    )); \
    return r; \
}

using strings::cat;

File::File(const char* path) : fd(PCALL(open)(path, O_RDONLY)) { }

File::~File() {
    ::close(fd);
}

RegularFile::RegularFile(const char* path): File(path) {
    struct stat sb;
    PCALL(fstat)(fd, &sb);
    if (!S_ISREG(sb.st_mode)) {
        throw std::runtime_error(cat(
            '\"', path, "\": not a regular file"
        ));
    }
    size = static_cast<size_t>(sb.st_size);
}

WholeFile::WholeFile(const char* path) {
    RegularFile f(path);

    size = f.size;
    m = static_cast<char*>(::malloc(f.size));
    if (!m) {
        const int e = errno;
        throw std::runtime_error(cat("malloc(): ", std::strerror(e)));
    }

    char* ptr = m;
    size_t need = size;
    while (need) {
        const auto n = ::read(f.fd, ptr, need);
        if (n < 0) [[ unlikely ]] {
            const int e = errno;
            ::free(m);
            m = nullptr;
            throw std::runtime_error(cat(
                "read(): \"", path, "\": ", std::strerror(e)
            ));
        }
        const size_t nsz = n;
        ptr += nsz;
        need -= nsz;
    }
}

WholeFile::~WholeFile() {
    ::free(m);
}

MappedFile::MappedFile(const char* path) {
    RegularFile f(path);

    size = f.size;
    m = reinterpret_cast<char*>(::mmap(
        0, f.size, PROT_READ, MAP_SHARED, f.fd, 0
    ));
    if (m == MAP_FAILED) [[ unlikely ]] {
        const int e = errno;
        m = nullptr;
        throw std::runtime_error(cat(
            "mmap(): \"", path, "\": ", std::strerror(e)
        ));
    }
}

MappedFile::~MappedFile() {
    if (m)
        ::munmap(m, size);
}

bool Exists(const char* path) noexcept {
  struct stat sb;
  return ::stat(path, &sb) == 0;
}
