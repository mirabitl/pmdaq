#pragma once

#include "rbProcessor.hh"
#include "irpcGeometry.hh"

#include <json/json.h>
#include "DCHistogramHandler.hh"


#include <map>
#include <vector>
#include <string>

namespace lmana
{

class rbRun;
class rbEvent;

class FebProcessor : public rbProcessor
{
public:

    FebProcessor();
    virtual ~FebProcessor();

    void init(uint32_t run=0) override;
    void end(uint32_t run=0) override;

    void processRunHeader(rbRun* r) override;
    void processEvent(rbEvent* e) override;

    void loadParameters(Json::Value params) override;

private:

    struct MappingChannel
    {
        uint32_t chan;
        uint32_t strip;
        uint32_t side;
        std::string name;
    };

    struct TdcChannel
    {
        uint32_t chan;
        uint32_t raw;
        double diff;
        uint32_t bc0id;
        double time;
        uint32_t strip;
        uint32_t side;
    };

    struct StripHit
    {
        uint32_t strip;

        double thr;
        double tlr;

        double zs;
        double xloc;
        double yloc;
    };

    void initializeMapping();
    void initializeMapping(const std::string& fname);

    uint32_t _run{0};
    uint32_t _nevt{0};
    double _tmin{-890.};
    double _tmax{-840.};

    uint64_t _nread{0};
    uint64_t _nfound{0};

    std::map<
        std::string,
        std::vector<MappingChannel>
    > _mapping;

    irpcGeometry _geo;
    DCHistogramHandler* _rh;
};

}

extern "C"
{
    lmana::rbProcessor* createProcessor();
}
