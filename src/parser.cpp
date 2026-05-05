#include "parser.hpp"

#include <cstdio>
#include <sstream>
#include <string_view>
#include <cstring>

#include <unistd.h>

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
    std::stringstream xml;
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

            xml << '<' << record.Name() << " size=\"" << record.size << '\"';
            if (record.flags[0] || record.flags[1])
                xml << " flags=\"" << std::hex
                    << record.flags[0] << ',' << record.flags[1]
                    << std::dec << '\"';
            xml << ">\n";

            for (const char *const end = ptr + record.size; ptr < end; ) {
                if (end - ptr < sizeof(Subrecord))
                    throw std::runtime_error("Not enough data for subrecord header");
                std::memcpy(&subrecord, ptr, sizeof(Subrecord));
                ptr += sizeof(Subrecord);
                if (end - ptr < subrecord.size)
                    throw std::runtime_error("Not enough data for subrecord data");
                ptr += subrecord.size;

                xml << "  <" << subrecord.Name() << " size=\"" << subrecord.size << '\"';
                xml << ">\n";


                xml << "  </" << subrecord.Name() << ">\n";
            }

            xml << "</" << record.Name() << ">\n";
        }
    }
    const auto view = xml.rdbuf()->view();
    write(1, view.data(), view.size());
}
