#include "Mmapped.hpp"
#include "Error.hpp"

#include <cstdio>
#include <cctype>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace strings {
using std::filesystem::path;
template <>
struct ViewADL<path> {
    static auto view(const path& p) noexcept {
        if constexpr (std::is_same_v<std::string, typename path::string_type>) {
            return std::string_view(p.native());
        } else {
            return p.string();
        }
    }
};
}

Mmapped::Mmapped(const std::filesystem::path& path) : path(path) {
    // https://www.oreilly.com/library/view/linux-system-programming/0596009585/ch04s03.html

    Error error;

    fd = ::open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        error(Error::code, "open(\"", path, "\")");
        goto err;
    }

    struct stat sb;
    if (::fstat(fd, &sb) == -1) {
        error(Error::code, "fstat(\"", path, "\")");
        goto close;
    }

    if (!S_ISREG(sb.st_mode)) {
        error('\"', path, "\" is not a regular file");
        goto close;
    }

    size = sb.st_size;

    m = reinterpret_cast<char*>(::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0));
    if (m == MAP_FAILED) {
        error(Error::code, "mmap(\"", path, "\")");
        goto close;
    }

    return;

close:
    if (::close(fd) == -1) {
        error(Error::code, "close(\"", path, "\")");
    }

err:
    throw error;
}

Mmapped::~Mmapped() {
    if (m != MAP_FAILED)
        ::munmap(m, size);
    if (fd != -1)
        ::close(fd);
}
