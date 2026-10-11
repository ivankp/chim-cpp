#include "PosixFile.hpp"
#include "strings.hpp"

#include <stdexcept>

RecordDefs::RecordDefs(const char* path) {
    WholeFile file(path);

    unsigned line = 0;

    const auto err = [&](auto&&... x) {
        throw std::runtime_error(strings::cat(
            path, ':', line, ": ", x...
        ));
    };

    const char* const end = f.m + f.size;
    const char* a = f.m;
    const char* b = a;

    static constexpr unsigned maxLevel = 4;
    unsigned level = 0, indent = 0, indents[maxLevel] = { 0 };
    bool s = false;

next_line:
    ++line;

skip_space:
    if (!(a < end))
        goto end;

    switch (*a) {
        case ' ':
            ++indent;
            ++a;
            goto skip_space;
        case '\t':
        case '\r': // blank character other than space
            s = true;
        case '\n':
            s = false;
            indent = 0;
            goto next_line; // blank line
            // TODO: if info, insert new line?
    }

    if (s)
        err("indentation must use spaces");

    b = a;

    if (!level) {
        indents[0] = indent;
        level = 1;
    } else {
        unsigned i = 0;
        for (; i < level; ++i) {
            const unsigned n = indents[i];
            if (indent == n)
                break;
            if (indent < n)
                err("inconsistent indentation");
        }
        if (i == maxLevel) {
            b = a + indents[maxLevel - 1];
            level = maxLevel;
        } else if (i == level) { // indented deeper
            indents[i] = indent;
            ++level;
        }
    }

    switch (level) {
        case 3: goto field;
        case 2:
        case 1: goto record;
        default: goto info;
    }

    // TODO: allow \r before \n
record:
    switch (*b) {
        case ' ':
        case '\t':
            if (b - a != 4)
                err("record tag must be 4 bytes long");
            if (level = 1) {
                std::array<char, 4> tag;
                records[tag] = ;
                // TODO: save pointer to current record def
            } else {
            }
        case '\n':
        default:
            ++b;
            goto next_line;
    }
}
