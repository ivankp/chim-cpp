#include "parser.hpp"
#include "PosixFile.hpp"
#include "strings.hpp"

#include <cstdio>
#include <sstream>
#include <string_view>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <map>

#include <unistd.h>

#include <set>
#include <fstream>

using strings::cat;

namespace {

constexpr char HexHalf(unsigned char half) noexcept {
    return half + (half < '\x0A' ? '0' : 'A' - '\x0A');
}

struct Hex {
    char c1, c2;
    constexpr Hex(unsigned char c) noexcept
    : c1(HexHalf(c >> 4)),
      c2(HexHalf(c & 0x0F))
    { }
};

const char* IsAsciiText(const char* a, const char* const end) {
    for (;; ++a) {
        if (!(a < end))
            return a;
        const char c = *a;
        if (c == '\0')
            break;
        if (c > '~' || (c < ' ' && c != '\n' && c != '\r' && c != '\t'))
            return nullptr;
    }
    for (const char* b = a;; ++b) {
        if (!(b < end))
            return a;
        const char c = *b;
        if (c != '\0')
            return nullptr;
    }
}

} // namespace

struct Subrecord {
    char name[4];
    uint32_t size;

    const char* ReadEs(const char* m, const char* end) {
        if (end - m < sizeof(Subrecord))
            throw std::runtime_error("Not enough data for subrecord header");
        std::memcpy(this, m, sizeof(Subrecord));
        m += sizeof(Subrecord);
        if (end - m < size)
            throw std::runtime_error("Not enough data for subrecord data");
        return m;
    }

    void WriteText(std::ostream& out) const {
        out << ' ';
        out.write(name, sizeof(name));
        out << ' ' << size << '\n';
    }
};

struct Record {
    char name[4];
    uint32_t size;
    uint32_t flags[2];

    const char* ReadEs(const char* m, const char* end) {
        if (end - m < sizeof(Record))
            throw std::runtime_error("Not enough data for record header");
        std::memcpy(this, m, sizeof(Record));
        m += sizeof(Record);
        if (end - m < size)
            throw std::runtime_error("Not enough data for record data");
        return m;
    }

    void WriteText(std::ostream& out) const {
        out.write(name, sizeof(name));
        out << ' ' << size << ' '
            << std::hex
            << flags[0] << ' ' << flags[1]
            << std::dec << '\n';
    }
};

void WriteAscii(std::ostream& out, const char* m, const char* end) {
    char c;
    for (; m < end; ++m) {
        c = *m;
next:
        if ('!' <= c && c <= '~' && c != '\\') { [[ likely ]]
            out << c;
        } else switch (c) {
            case ' ':
                for (;;) {
                    if (++m == end) {
                        out << "\\s";
                        return;
                    }
                    out << ' ';
                    c = *m;
                    if (c != ' ') {
                        goto next;
                    }
                }
            case '\n':
                out << "\\n";
                if (m + 1 == end)
                    return;
                out << "\n   ";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            case '\\':
                out << "\\\\";
                break;
            default:
                out << "\\";
                const Hex h = c;
                out << h.c1 << h.c2;
        }
    }
}

std::map<std::string, std::set<std::string>> tags;

struct Def {
    std::string_view type, desc;
};

const auto [ defs, defsData ] = [] {
    std::map<std::array<char, 8>, std::vector<Def>> all;
    std::vector<Def>* defs = nullptr;

    const char* filename = "TES3.defs";
    WholeFile f(filename);
    const char* const end = f.m + f.size;
    const char* a = f.m;
    const char* b = a;

    unsigned line = 0;
    unsigned indent[3] { };
    unsigned level = 0;
    std::array<char, 8> tag;

    for (; b < end; ++b) {
        if (*b == ' ')
            ++indent[level];
        else
            break;
    }
    a = b;
    for (; b < end; ++b) {
        switch (*b) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                goto tag;
        }
    }
tag:
    if (level <= 1) {
        if (b - a != 4) throw std::runtime_error(cat(
            filename, ':', line, ": invalid tag"
        ));
        std::memcpy(tag.data() + level * 4, a, 4);
    } else {
        defs->push_back({ { a, b }, { } });
    }

    return std::tuple(all, std::move(f));
}();

Mod::Mod(const std::filesystem::path& path) : Mmapped(path) {
    char hexLine[8 * (8 + 1) + 3];
    std::memset(hexLine, ' ', sizeof(hexLine)-1);
    hexLine[sizeof(hexLine) - 1] = '\n';

    std::stringstream out;
    if (std::string_view(m, size).starts_with("TES3")) {
        Record record;
        Subrecord subrecord;
        const char *ptr = m;
        const char* const endFile = m + size;
        while (ptr < endFile) {
            ptr = record.ReadEs(ptr, endFile);
            record.WriteText(out);

            auto& tagsRec = tags[std::string(record.name, 4)];

            const char* const endRec = ptr + record.size;
            while (ptr < endRec) {
                ptr = subrecord.ReadEs(ptr, endRec);
                subrecord.WriteText(out);

                tagsRec.insert(std::string(subrecord.name, 4));

                if (subrecord.size == 0)
                    continue;

                const char* const endSub = ptr + subrecord.size;
                if (const char* endTxt = IsAsciiText(ptr, endSub)) {
                    out << "  str\n   ";
                    if (ptr != endTxt) {
                        WriteAscii(out, ptr, endTxt);
                    } else { [[ unlikely ]]
                        out << "\\0";
                    }
                    out << '\n';
                    ptr += subrecord.size;
                } else {
                    out << "  hex\n";
                    unsigned i = 2;
                    if (ptr < endSub) for (;;) {
                      const Hex h = *ptr;
                      hexLine[++i] = h.c1;
                      hexLine[++i] = h.c2;
                      if (++ptr == endSub) {
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

    std::ofstream tagsOut("tags.txt");
    for (const auto& [rec, subs] : tags) {
        tagsOut << rec << '\n';
        for (const auto& sub : subs) {
            tagsOut << "  " << sub << '\n';
        }
    }
}
