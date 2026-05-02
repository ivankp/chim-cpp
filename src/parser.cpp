#include "parser.hpp"
// #include "strings.hpp"

#include <iostream>
// #include <vector>
#include <cstdio>
#include <generator>

// using strings::cat;
//
// void Print(const auto&... args) {
//     std::puts(cat(args...).c_str());
// }

using std::cout, std::endl;

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
    uint32_t header;
    uint32_t flags;

    std::string_view Name() const noexcept {
        return { name, sizeof(name) };
    }

    std::generator<Subrecord&> Subrecords() const {
        char* ptr = reinterpret_cast<char*>(const_cast<Record*>(this)) + sizeof(Record);
        const char* const end = ptr + size;
        while (ptr < end) {
            if (end - ptr < sizeof(Subrecord))
                throw std::runtime_error("Not enough data for subrecord header");
            Subrecord* subrecord = reinterpret_cast<Subrecord*>(ptr);
            ptr += sizeof(Subrecord);
            if (end - ptr < subrecord->size)
                throw std::runtime_error("Not enough data for subrecord data");
            ptr += subrecord->size;
            co_yield *subrecord;
        }
    }
};

std::generator<Record&> Records(char* ptr, const char* end) {
    while (ptr < end) {
        // if (reinterpret_cast<std::uintptr_t>(ptr) % alignof(Record) != 0) {
        //     throw std::runtime_error("Record pointer is not aligned");
        // }
        if (end - ptr < sizeof(Record))
            throw std::runtime_error("Not enough data for record header");
        Record* record = reinterpret_cast<Record*>(ptr);
        ptr += sizeof(Record);
        if (end - ptr < record->size)
            throw std::runtime_error("Not enough data for record data");
        ptr += record->size;
        co_yield *record;
    }
}

Mod::Mod(const std::filesystem::path& path) : Mmapped(path) {
    if (std::string_view(m, size).starts_with("TES3")) {
        const char* fileEnd = m + size;
        for (Record& record : Records(m, m + size)) {
            // Print(record.Name(), ' ', record.size);
            cout << '<' << record.Name() << " size=\"" << record.size << '\"';
            if (record.header != 0)
                cout << " header=\"" << std::hex << record.header << std::dec << '\"';
            if (record.flags != 0)
                cout << " flags=\"" << std::hex << record.flags << std::dec << '\"';
            cout << ">\n";

            for (Subrecord& subrecord : record.Subrecords()) {
                // Print("  ", subrecord.Name(), ' ', subrecord.size);
                cout << "  <" << subrecord.Name() << " size=\"" << subrecord.size << '\"';
                cout << ">\n";


                cout << "  </" << subrecord.Name() << ">\n";
            }

            cout << "</" << record.Name() << ">\n";
        }
    }
}
