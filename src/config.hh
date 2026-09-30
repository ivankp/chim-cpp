#include <array>
#include <map>
#include <string_view>

struct FieldDef {
    std::string_view type, name, info;
};

struct SubrecordDef {
    std::string_view info;
    std::vector<FieldDef> fields;
};

struct RecordDefs {
    std::map<std::array<char, 4>, std::string_view> records;
    std::map<std::array<char, 8>, SubrecordDef> subrecords;

    RecordDefs(const char* path);
};
