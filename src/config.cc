#include "PosixFile.hpp"

#include <stdexcept>

RecordDefs::RecordDefs(const char* path) {
    WholeFile file(path);

    const char* const end = f.m + f.size;
    const char* a = f.m;
    const char* b = a;

    unsigned level = 0, i = 0, indent[3] = { 0 };
    bool s = false;

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
            i = 0;
            goto skip_space; // blank line
    }

    if (s)
        goto err_indent_char;

    if (!level) {
        indent[0] = i;
    } else {
    }

    b = a;

next:
    switch (*b) {
        case ' ':
        case '\t':
            if (!s) {
                s = true;
                if (b - a != 4)
                    goto err_tag_length;

            } else {
            }
        case '\n':
        default:
            ++b;
            goto next;
    }



err_indent_char:
    throw std::runtime_error("Record definition indentation must use spaces");
err_tag_length:
    throw std::runtime_error("Record tag must be 4 characters long");
}
