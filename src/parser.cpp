#include "parser.hpp"

#include <cstdio>
#include <sstream>
#include <string_view>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <unistd.h>

using std::cout;

struct Subrecord {
    char name[4];
    uint32_t size;

    std::string_view Name() const noexcept {
        return { name, sizeof(name) };
    }
};

struct Record {
    char name[4];
    uint32_t size;
    uint32_t flags[2];

    std::string_view Name() const noexcept {
        return { name, sizeof(name) };
    }
};

Mod::Mod(const std::filesystem::path& path) : Mmapped(path) {
    char hexLine[8 * (8 + 1) + 3];
    std::memset(hexLine, ' ', sizeof(hexLine)-1);
    hexLine[sizeof(hexLine) - 1] = '\n';

    std::stringstream out;
    if (std::string_view(m, size).starts_with("TES3")) {
        Record record;
        Subrecord subrecord;
        for (const char *ptr = m, *const end = m + size; ptr < end; ) {
            if (end - ptr < sizeof(Record))
                throw std::runtime_error("Not enough data for record header");
            std::memcpy(&record, ptr, sizeof(Record));
            ptr += sizeof(Record);
            if (end - ptr < record.size)
                throw std::runtime_error("Not enough data for record data");

            out << record.Name() << ' ' << record.size << ' '
                << std::hex
                << record.flags[0] << ' ' << record.flags[1]
                << std::dec << '\n';

            for (const auto end = ptr + record.size; ptr < end; ) {
                if (end - ptr < sizeof(Subrecord))
                    throw std::runtime_error("Not enough data for subrecord header");
                std::memcpy(&subrecord, ptr, sizeof(Subrecord));
                ptr += sizeof(Subrecord);
                if (end - ptr < subrecord.size)
                    throw std::runtime_error("Not enough data for subrecord data");

                out << ' ' << subrecord.Name() << ' ' << subrecord.size << '\n';

                const char* const dataEnd = ptr + subrecord.size;
                const char* dataEnd2 = dataEnd - 1; // without trailing \0
                for (; dataEnd2 >= ptr; --dataEnd2) {
                    if (*dataEnd2 != '\0') {
                        break;
                    }
                }
                ++dataEnd2;

                if (dataEnd2 - ptr > 2 && std::all_of(ptr, dataEnd2, [](char c) {
                    return ('\x19' < c && c < '\x7F')
                        || c == ' ' || c == '\t' || c == '\n' || c == '\r';
                })) { // Text
                    out << "  str\n   ";
                    out.write(ptr, subrecord.size);
                    out << '\n';
                    ptr += subrecord.size;
                } else {
                    out << "  hex\n";
                    unsigned i = 2;
                    if (ptr < dataEnd) for (;;) {
                        const uint8_t byte = *reinterpret_cast<const uint8_t*>(ptr);
                        uint8_t half = byte >> 4;
                        hexLine[++i] = (half < 10 ? '0' + half : 'A' - 10 + half);
                        half = byte & 0x0F;
                        hexLine[++i] = (half < 10 ? '0' + half : 'A' - 10 + half);
                        if (++ptr == dataEnd) {
                            out.write(hexLine, i + 1) << '\n';
                            break;
                        }
                        if (!((i - 1) % 9))
                            ++i;
                        if (i == sizeof(hexLine) - 1) {
                            out.write(hexLine, sizeof(hexLine));
                            i = 2;
                        }
                    }
                }
            }
        }
    }
    const auto view = out.rdbuf()->view();
    write(1, view.data(), view.size());
}
