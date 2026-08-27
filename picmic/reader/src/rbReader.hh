#pragma once

#include <string>

namespace lmana
{

class rbProcessor;

class rbReader
{
public:

    void read(const std::string& fileName,
              rbProcessor* processor);
};

}
