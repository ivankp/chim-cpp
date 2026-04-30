#include "parser.hpp"
#include "strings.hpp"

// #include <iostream>
// #include <vector>
#include <cstdio>

using strings::cat;

void Print(const auto&... args) {
    std::puts(cat(args...).c_str());
}

struct Record {
    char name[4];
    uint32_t size;
    uint32_t header;
    uint32_t flags;

    std::string_view Name() const noexcept {
        return { name, sizeof(name) };
    }

    static const Record* Cast(const char* ptr, const char* end) {
        if (ptr == end)
            return nullptr;
        if (end - ptr < sizeof(Record))
            throw std::runtime_error("Not enough data for record header");
        const Record* record = reinterpret_cast<const Record*>(ptr);
        if (end - (ptr + sizeof(Record)) < record->size)
            throw std::runtime_error("Not enough data for record data");
        return record;
    }

    const Record* Next(const char* end) const {
        return Cast(reinterpret_cast<const char*>(this) + sizeof(Record) + size, end);
    }
};

struct Subrecord {
    char name[4];
    uint32_t size;
};

Mod::Mod(const std::filesystem::path& path) : Mmapped(path) {

    if (std::string_view(m, size).starts_with("TES3")) {
        const char* fileEnd = m + size;
        for (
            const Record* record = Record::Cast(m, fileEnd);
            record; record = record->Next(fileEnd)
        ) {
            // std::cout.write(record->name, 4) << ' ' << record->size << std::endl;
            Print(record->Name(), ' ', record->size);
        }
    }
}
