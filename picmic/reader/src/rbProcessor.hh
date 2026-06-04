#pragma once

#include <string>
#include <json/json.h>

namespace lmana
{

class rbEvent;
class rbRun;

class rbProcessor
{
public:

    virtual ~rbProcessor() = default;

    virtual void init(uint32_t run=0)=0;
    virtual void end(uint32_t run=0)=0;

    virtual void processEvent(rbEvent* e)=0;
    virtual void processRunHeader(rbRun* r)=0;

    virtual void loadParameters(Json::Value params)=0;

    inline void setName(const std::string& n)
    {
        _name=n;
    }

    inline std::string name() const
    {
        return _name;
    }

private:
    std::string _name;
};

}
