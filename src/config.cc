#include "config.hh"
#include "strings.hpp"

#include <stdexcept>
#include <fstream>

using strings::cat;

namespace {

void SkipSpace(const char*& b, const char* end) {
next:
    if (b != end) {
        switch (*b) {
            case ' ':
            case '\t':
                ++b;
                goto next;
        }
    }
}

void SkipWord(const char*& b, const char* end) {
next:
    if (b != end) {
        switch (*b) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                break;
            default:
                ++b;
                goto next;
        }
    }
}

} // namespace

RecDefs::RecDefs(const char* path) : file(path) {
    unsigned line = 0;

    const auto err = [&](auto&&... x) {
        throw std::runtime_error(cat(
            path, ':', line, ": ", x...
        ));
    };

    const char* const end = file.m + file.size;
    const char* a = file.m;
    const char* b = a;
    const char* lineEnd;

    static constexpr unsigned maxLevel = 3;
    unsigned indent = 0;

    RecTag recTag;
    SubrecTag subTag;

    std::string_view* recInfo = nullptr;
    SubrecDef* subrec = nullptr;
    FieldDef* field = nullptr;

next_line:
    ++line;

    // Count leading tabs
    for (;; ++b) {
        if (b == end)
            return;
        if (*b != '\t')
            break;
    }
    indent = b - a;

    // Ignore blank lines
    for (;; ++b) {
        if (b == end)
            return;
        switch (*b) {
            case '\n':
                a = ++b;
                goto next_line;
            case ' ':
            case '\t':
            case '\r':
                continue;
        }
        break;
    }

    if (indent > maxLevel) {
        indent = maxLevel;
    }
    a += indent;
    b = a;

    switch (indent) {
        case 0u:
        case 1u: goto record;
        case 2u: goto field;
        default: goto info;
    }

record:
    SkipWord(b, end);
    if (b - a != 4)
        err("record tag must be 4 bytes long");

    if (!indent) {
        std::memcpy(recTag.data(), a, 4);
        std::memcpy(subTag.data(), a, 4);
        recInfo = &recs[recTag];
        subrec = nullptr;
    } else {
        if (!recInfo) {
            err("subrecord without record");
        }
        std::memcpy(subTag.data() + 4, a, 4);
        subrec = &subrecs[subTag];
    }
    field = nullptr;

    SkipSpace(b, end);
    a = b;

    goto info;

field:
    if (!subrec) {
        err("field without subrecord");
    }

    field = &subrec->fields.emplace_back();

    SkipWord(b, end);
    field->type = { a, b };
    SkipSpace(b, end);
    a = b;

    SkipWord(b, end);
    field->name = { a, b };
    SkipSpace(b, end);
    a = b;

    goto info;

info:
    // find end of the line
    if (b == end) {
        lineEnd = b;
    } else if (*b == '\n') {
        lineEnd = b;
        if (b > a && b[-1] == '\r') {
            --lineEnd;
        }
    } else {
        ++b;
        goto info;
    }
    // strip trailing whitespace
    while (--lineEnd > a) {
        switch (*lineEnd) {
            case ' ':
            case '\t':
                break;
            default:
                goto line_end;
        }
    }
line_end:
    ++lineEnd;

    std::string_view tail(a, lineEnd);
    switch (indent) {
        case 0u:
            *recInfo = tail;
            break;
        case 1u:
            subrec->info = tail;
            break;
        default:
            if (!field) {
                err("info without field");
            }
            auto& info = field->info;
            if (indent > 2)
                info += '\n';
            info += tail;
    }

    if (b != end) {
        a = ++b;
        goto next_line;
    }
}

void RecDefs::Write(const char* path) const {
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error(cat("Failed to open ", path));

    auto subrec = subrecs.begin();
    auto subEnd = subrecs.end();

    for (const auto& [ recTag, recInfo ] : recs) {
        out.write(recTag.data(), 4);
        if (!recInfo.empty()) {
            out << ' ' << recInfo;
        }
        out << '\n';
        for (; subrec != subEnd; ++subrec) {
            const auto& [ subTag, subDef ] = *subrec;
            for (unsigned i = 0; i < 4; ++i) {
                if (recTag[i] != subTag[i])
                    goto next_tag;
            }
            out << '\t';
            out.write(subTag.data() + 4, 4);

            const auto& [ subInfo, fields ] = subDef;
            if (!subInfo.empty()) {
                out << ' ' << subInfo;
            }
            out << '\n';

            for (const auto& field : fields) {
                out << "\t\t" << field.type << ' ' << field.name;
                if (!field.info.empty()) {
                    out << ' ';
                    for (char c : field.info) {
                        out << c;
                        if (c == '\n') {
                            out << "\t\t\t";
                        }
                    }
                }
                out << '\n';
            }
        }
next_tag:;
    }
}
