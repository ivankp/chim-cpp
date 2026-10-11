#pragma once

#include "PosixFile.hpp"

#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <map>

struct FieldDef {
    std::string_view type, name;
    std::string info;
};

struct SubrecDef {
    std::string_view info;
    std::vector<FieldDef> fields;
};

using RecTag = std::array<char, 4>;
using SubrecTag = std::array<char, 8>;

struct RecDefs {
    WholeFile file;
    std::map<RecTag, std::string_view> recs;
    std::map<SubrecTag, SubrecDef> subrecs;

    RecDefs(const char* path);
    void Write(const char* path) const;
};
