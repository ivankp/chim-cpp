#include "parser.hpp"
#include "strings.hpp"

#include <filesystem>
#include <exception>
#include <type_traits>

#include <cstdio>

using std::filesystem::path;
using strings::cat;

namespace strings {

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

int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            std::fputs(cat(
                "usage: ", path(argv[0]).filename(), " input_file\n"
            ).c_str(), stderr);
            return 1;
        }

        const ParsedMod mod(argv[1]);

    } catch (...) {
        std::fputs("Terminated due to exception\n", stderr);
        try {
            throw;
        } catch (const std::exception& e) {
            std::fputs(cat(e.what(), '\n').c_str(), stderr);
        }
    }
}
