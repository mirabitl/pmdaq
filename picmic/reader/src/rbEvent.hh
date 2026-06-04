#pragma once

#include <cstdint>
#include <vector>

namespace lmana
{

class rbEvent
{
public:
    uint32_t eventNumber{0};
    uint64_t timestamp{0};

    std::vector<std::vector<uint64_t>> blocks;
};

}
