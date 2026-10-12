#include "parser.hh"
#include "strings.hh"

#include <filesystem>
#include <exception>
#include <type_traits>

#include <cstdio>

namespace fs = std::filesystem;
using strings::cat;

namespace strings {
template <>
struct ViewADL<fs::path> {
    static auto view(const fs::path& p) noexcept {
        if constexpr (std::is_same_v<std::string, typename fs::path::string_type>) {
            return std::string_view(p.native());
        } else {
            return p.string();
        }
    }
};
}

int main(int argc, char** argv) {
    try {
        if (argc != 2 && argc != 3) {
            std::fputs(cat(
                "usage: ", fs::path(argv[0]).filename(),
                " input_file [output_file]\n"
            ).c_str(), stderr);
            return 1;
        }

        EsToChim(
            argv[1],
            argc > 2 ? argv[2] :
                fs::path(argv[1]).filename().replace_extension(".chim").c_str()
        );

    } catch (...) {
        std::fputs("Terminated due to exception\n", stderr);
        try {
            throw;
        } catch (const std::exception& e) {
            std::fputs(cat(e.what(), '\n').c_str(), stderr);
        }
    }
}
