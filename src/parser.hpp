#pragma once

#include "Mmapped.hpp"

class Mod : Mmapped {
public:
    Mod(const std::filesystem::path&);
};
