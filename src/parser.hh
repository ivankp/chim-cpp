#pragma once

#include "Mmapped.hh"

class Mod : Mmapped {
public:
    Mod(const std::filesystem::path&);
};
