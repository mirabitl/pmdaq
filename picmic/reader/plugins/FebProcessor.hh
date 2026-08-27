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

  class FebCluster
  {
  public:
    FebCluster(std::vector<std::shared_ptr<StripHit> > v)
    {_hits=std::move(v);}
    inline uint32_t size(){return _hits.size();}
    inline double t(){return _hits[0]->thr;}
    inline double x(){return _hits[0]->xloc;}
    inline double y(){return _hits[0]->yloc;}
    inline double zs(){return _hits[0]->zs;}
    inline  uint32_t strip(){ return _hits[0]->strip;}
    inline std::vector<std::shared_ptr<StripHit> >& hits(){return _hits;}
  private:
    std::vector<std::shared_ptr<StripHit> > _hits;
  };

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
  


  void initializeMapping();
  void initializeMapping(const std::string& fname);
  std::vector<std::vector<std::shared_ptr<StripHit>>> clusteriserHits(const std::vector<std::shared_ptr<StripHit>>& hits);

  
    uint32_t _run{0};
    uint32_t _nevt{0};
    double _tmin{-890.};
    double _tmax{-840.};

    uint64_t _nread{0};
    uint64_t _nfound{0};
    uint64_t _nstrip{0};
  std::vector<std::shared_ptr<FebCluster>> _clusters;
    std::map<
        std::string,
        std::vector<MappingChannel>
    > _mapping;

    irpcGeometry _geo;
    DCHistogramHandler* _rh;
  Json::Value _params;
};

}

extern "C"
{
    lmana::rbProcessor* createProcessor();
}
