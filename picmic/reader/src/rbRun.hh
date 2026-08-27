#pragma once

#include <cstdint>
#include <vector>
#include <json/json.h>

namespace lmana
{

class rbRun
{
public:
    uint32_t runNumber{0};
    uint64_t timestamp{0};

    bool isJson{false};

    std::vector<uint32_t> rawHeader;
    Json::Value jsonHeader;
};

}
