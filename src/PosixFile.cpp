#include "PosixFile.hpp"

#include "strings.hpp"

#include <cstdlib>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

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

File::File(const char* name) : fd(PCALL(open)(name, O_RDONLY)) { }

File::~File() {
    ::close(fd);
}

RegularFile::RegularFile(const char* name): File(name) {
    struct stat sb;
    PCALL(fstat)(fd, &sb);
    if (!S_ISREG(sb.st_mode)) throw std::runtime_error(cat(
        '\"', name, "\": not a regular file"
    ));
    size = static_cast<size_t>(sb.st_size);
}

WholeFile::WholeFile(const char* name) {
    RegularFile f(name);

    m = static_cast<char*>(::malloc(f.size));
    if (!m) throw std::runtime_error(cat(
        "malloc(): ", std::strerror(errno)
    ));
    size = f.size;
    for (size_t need = size; need; ) {
        const auto n = ::read(f.fd, m, need);
        if (n < 0) [[ unlikely ]] {
            const auto e = errno;
            ::free(m);
            m = nullptr;
            throw std::runtime_error(cat(
                "read(): \"", name, "\": ", std::strerror(e)
            ));
        }
        need -= static_cast<size_t>(n);
    }
}

WholeFile::~WholeFile() {
    ::free(m);
}

bool Exists(const char* name) noexcept {
  struct stat sb;
  return ::stat(name, &sb) == 0;
}
