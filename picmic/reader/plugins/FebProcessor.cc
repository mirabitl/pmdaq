#include "FebProcessor.hh"

#include "rbRun.hh"
#include "rbEvent.hh"

#include "TdcUplinkFrame.hh"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

using namespace lmana;

namespace
{
  constexpr double ORB_LEN = 92175.0;

    static const std::string FPGA_NAME[3] =
    {
        "LEFT",
        "MIDDLE",
        "RIGHT"
    };
}

////////////////////////////////////////////////////////////////
/// ctor / dtor
////////////////////////////////////////////////////////////////

FebProcessor::FebProcessor()
{
    setName("FebProcessor");
    _rh=DCHistogramHandler::instance();
	_nevt=0;
}

FebProcessor::~FebProcessor()
{
}

void FebProcessor::initializeMapping()
{
  if (!_params.isMember("mapping"))
    {
      std::cerr << "No mapping file exiting " << _params << "'\n";
      exit(-1);
    }
  initializeMapping(_params["mapping"].asString()); // No extender merge PCB
    //initializeMapping("../etc/extender.json"); //extender PCB

}

void FebProcessor::initializeMapping(const std::string& fname)
{
    _mapping.clear();
    for (auto const& name : FPGA_NAME)
        _mapping[name].assign(34, MappingChannel());

    std::ifstream file(fname);
    if (!file)
    {
        std::cerr << "FebProcessor::initializeMapping: cannot open mapping file '" << fname << "'\n";
        return;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::Value root;
    std::string errors;

    if (!Json::parseFromStream(builder, file, &root, &errors))
    {
        std::cerr << "FebProcessor::initializeMapping: JSON parse error: " << errors << '\n';
        return;
    }

    const Json::Value& content = root["content"];
    if (!content.isObject())
    {
        std::cerr << "FebProcessor::initializeMapping: JSON mapping file missing 'content' object\n";
        return;
    }

    for (const auto& key : content.getMemberNames())
    {
        const Json::Value& value = content[key];
        if (!value.isArray() || value.size() < 4)
            continue;

        std::string board = value[0].asString();
        std::string position = value[1].asString();
        uint32_t channel = value[3].asUInt();

        std::string fpgaKey;
        if (board == "left")
            fpgaKey = "LEFT";
        else if (board == "middle")
            fpgaKey = "MIDDLE";
        else if (board == "right")
            fpgaKey = "RIGHT";
        else
            continue;

        uint32_t side =  (key[0]=='r') ? 0 : 1;

        uint32_t strip = 0;
        auto posUnd = key.rfind('_');
        if (posUnd != std::string::npos)
            strip = static_cast<uint32_t>(std::stoul(key.substr(posUnd + 1)));

        if (channel >= _mapping[fpgaKey].size())
            _mapping[fpgaKey].resize(channel + 1, MappingChannel());
        //std::cout<<key<<" "<<value[0].asString()<<" "<<value[1].asString()<<" "<<value[2].asString()<<" "<<value[3].asUInt()<<std::endl;   
        std::cout << "Mapping: " << key << " -> FPGA " << fpgaKey << " channel " << channel << " strip " << strip << " side " << side << std::endl;
        _mapping[fpgaKey][channel] = MappingChannel{channel, strip, side, key};
    }
    //getchar();
}

////////////////////////////////////////////////////////////////
/// init
////////////////////////////////////////////////////////////////

void FebProcessor::init(uint32_t)
{
    initializeMapping();

    _tmin = -950.;
    _tmax = -840.;
    if (_params.isMember("tmin")) _tmin=_params["tmin"].asDouble();
    if (_params.isMember("tmax")) _tmax=_params["tmax"].asDouble();

    //std::cout<<_tmin<<" "<<_tmax<<std::endl;
    //getchar();
    //
    // Géométrie
    //
    if (!_params.isMember("geometry"))
    {
      std::cerr << "No geometry file exiting " << _params << "'\n";
      exit(-1);
    }

    {
      std::ifstream geoFile(_params["geometry"].asString());
        if (!geoFile)
        {
	  std::cerr << "FebProcessor::init: cannot open geometry "<<_params["geometry"].asString()<<"\n";
	  getchar();
        }
        else
        {
            Json::CharReaderBuilder builder;
            builder["collectComments"] = false;
            Json::Value root;
            std::string errors;

            if (!Json::parseFromStream(builder, geoFile, &root, &errors))
            {
                std::cerr << "FebProcessor::init: JSON parse error in etc/RE31_1.json: " << errors << '\n';
		getchar();
            }
            else if (!root.isObject() || !root["content"].isObject() || !root["content"]["LEFT"].isObject())
            {
                std::cerr << "FebProcessor::init: invalid geometry JSON format in etc/RE31_1.json\n";
		getchar();
            }
            else
            {
	      _geo.initialize(_params["geotag"].asString(), root["content"][_params["geoside"].asString()]);
            }
        }
    }

    //
    // Histos
    //
}
////////////////////////////////////////////////////////////////
/// loadParameters
////////////////////////////////////////////////////////////////

void FebProcessor::loadParameters(Json::Value p)
{
  _params=p;
}

////////////////////////////////////////////////////////////////
/// run header
////////////////////////////////////////////////////////////////

void FebProcessor::processRunHeader(rbRun* r)
{
    _run = r->runNumber;


    std::cout
        << "Run "
        << _run
        << std::endl;
}

////////////////////////////////////////////////////////////////
/// event
////////////////////////////////////////////////////////////////

void FebProcessor::processEvent(rbEvent* e)
{
  _clusters.clear();
    auto _hstat = _rh->AccessTH1("statistic", 40, 0, 40);
    _hstat->Fill(1);
    if (_nread % 20 == 0 && _nread>5)
      {
	auto neff=3560.*_nread/(3560+126);
        std::cout
            << "Event "
            << e->eventNumber
            << " timestamp "
            << e->timestamp
            << " read " << _nread << " events, found " << _nfound << " FEB hits"<<" Efficiency: " 
            << (_nfound * 100.0 / neff) << "%  Multiplicity " << 1.*_nstrip/_nfound <<"\n";
      }
    _nread++;     
    std::vector<TdcChannel> channels;
    std::vector<std::shared_ptr<StripHit>> strips;

    //----------------------------------------------------------
    // tdcdata[fpga]
    //----------------------------------------------------------

    std::map<
        std::string,
        std::vector<
            std::tuple<uint32_t,uint32_t,uint32_t>
        >
    > tdcdata;

    //----------------------------------------------------------
    // Decode frames
    //----------------------------------------------------------

    for (const auto& block : e->blocks)
    {
        for (size_t i=0;i+7<block.size();i+=8)
        {
            TdcUplinkFrame::FrameWords fw;

            for (size_t k=0;k<8;k++)
                fw[k]=static_cast<uint32_t>(block[i+k]);

            TdcUplinkFrame frame(fw);

            if (frame.feb0_scframe())
                continue;

            auto addHit =
                [&](uint32_t fpga,
                    uint32_t chan,
                    uint32_t tdc,
                    uint32_t bc0)
            {
	      // Juste middle
	      //if (fpga!=0) return;
                if (fpga>2)
                    return;

                tdcdata[
                    FPGA_NAME[fpga]
                ].push_back(
                    {chan,tdc,bc0});
            };

            if (frame.feb0_dvalid_0())
                addHit(
                    frame.feb0_devaddr_0(),
                    frame.feb0_chanid_0(),
                    frame.feb0_tdc_data_0(),
                    frame.bc0id());

            if (frame.feb0_dvalid_1())
                addHit(
                    frame.feb0_devaddr_1(),
                    frame.feb0_chanid_1(),
                    frame.feb0_tdc_data_1(),
                    frame.bc0id());

            if (frame.feb0_dvalid_2())
                addHit(
                    frame.feb0_devaddr_2(),
                    frame.feb0_chanid_2(),
                    frame.feb0_tdc_data_2(),
                    frame.bc0id());
        }
    }

    //----------------------------------------------------------
    // reference t0 (channel 33)
    //----------------------------------------------------------

    double t0=0.;
    bool haveT0=false;

    for (const auto& fpga : tdcdata)
    {
        std::vector<std::vector<double>>
            chan_ts(34);

        for (auto const& h : fpga.second)
        {
            uint32_t chan =
                std::get<0>(h);

            uint32_t ts =
                std::get<1>(h);

            uint32_t bc0 =
                std::get<2>(h);

            double tc =
                (bc0-1)*ORB_LEN +
                ts*2.5/256.;

            if (chan<34)
                chan_ts[chan].push_back(tc);
        }

        if (!chan_ts[33].empty())
        {
            t0=chan_ts[33][0];
            haveT0=true;
            break;
        }
    }

    if (!haveT0)
        return;

    //----------------------------------------------------------
    // build channels
    //----------------------------------------------------------

    bool found=false;
    bool strip_found=false;
    bool flow=false;
    bool fhigh=false;

    for (const auto& fpga : tdcdata)
    {
      //printf("FPGA: %s with %zu hits\n", fpga.first.c_str(), fpga.second.size());
      //if (fpga.first!="MIDDLE")
      //      continue;
        auto itMap =
            _mapping.find(fpga.first);

        if (itMap==_mapping.end())
            continue;
	//printf("On passe la \n");
	int nchok=0;
        for (auto const& h : fpga.second)
        {
            uint32_t chan =
                std::get<0>(h);

            uint32_t ts =
                std::get<1>(h);

            uint32_t bc0 =
                std::get<2>(h);

            double ctime =
                (bc0-1)*ORB_LEN +
                ts*2.5/256.;

            double diff =
                ctime - t0;

	    //printf(" FPGA %s Channel %d  \n",fpga.first.c_str(),chan); 
            if (chan < 32)
            {
                const auto& m =
                    itMap->second[chan];
		//printf(" FPGA %s Channel %d Strip %d Side %d \n",fpga.first.c_str(),chan,m.strip,m.side);
		nchok++;
                channels.push_back(
                {
                    chan,
                    ts,
                    diff,
                    bc0,
                    ctime,
                    m.strip,
                    m.side
                });
                auto _htma = _rh->AccessTH1("/DT/All/"+fpga.first+"/channel_" + std::to_string(chan),3000,-30000.,0.);
                
                _htma->Fill(diff);
                if (diff>_tmin &&
                    diff<_tmax)
                {
                    found=true;
                    auto _hch = _rh->AccessTH1("chan", 96, 0, 96);
		    uint32_t cshift=0;
		    if (fpga.first=="MIDDLE")
		      cshift=32;
		    if (fpga.first=="RIGHT")
		      cshift=64;
                    _hch->Fill(chan+cshift);

                    if (chan<16)
                        fhigh=true;
                    else
                        flow=true;
                    auto _htm = _rh->AccessTH1("/DT/"+fpga.first+"/channel_" + std::to_string(chan),
                    150,_tmin-50,_tmax+50 );
                    _htm->Fill(diff);
                }
            }
        }
	//printf("On passe ici %d \n",nchok);
    }

    //getchar();
    //----------------------------------------------------------
    // strip pairing
    //----------------------------------------------------------

    std::sort(
        channels.begin(),
        channels.end(),
        [](const TdcChannel& a,
           const TdcChannel& b)
        {
            if (a.strip!=b.strip)
                return a.strip<b.strip;

            return a.diff>b.diff;
        });

    for (size_t i=0;i<channels.size();i++)
    {
        auto const& hi = channels[i];

        if (hi.side!=0)
            continue;

        if (hi.diff<_tmin ||
            hi.diff>_tmax)
            continue;

        for (size_t j=i+1;
             j<channels.size();
             j++)
        {
            auto const& lo =
                channels[j];

            if (lo.strip!=hi.strip)
                break;

            if (lo.side!=1)
                continue;

            if ((hi.diff-lo.diff)>30.)
                continue;

            double zs;
            double x;
            double y;
	    auto res=_geo.getLocalPosition(47-lo.strip,hi.diff,lo.diff);
	    if (!res.valid) continue;

            //_geo.localPosition(
	    // lo.strip,
	    // hi.diff,
	    // lo.diff,
	    // zs,
	    // x,
	    // y);
	    StripHit hit{
	      lo.strip,
	      hi.diff,
	      lo.diff,
	      res.zs,
	      res.position.x,
	      res.position.y
	    };
	    auto st=std::make_shared<StripHit>(hit);
            strips.push_back(st);
            auto _hdiff = _rh->AccessTH1("hdiff",200,-50.,150.);
            auto _hstrip = _rh->AccessTH1("strip", 50, 0, 50);
            auto _hzs = _rh->AccessTH1("zs", 200, -50, 150);
            auto _hxy = _rh->AccessTH2("xy", 90,-20.,160.,30,0.,60.);
            auto _hpos = _rh->AccessTH2("pos", 90,-20.,160.,50,0.,50.);
            _hdiff->Fill(
                hi.diff-lo.diff);

            _hstrip->Fill(
                lo.strip);

            _hzs->Fill(res.zs);

            _hxy->Fill(res.position.y,res.position.x);

            _hpos->Fill(res.zs,lo.strip);

        }
    }
    auto _hnstrip = _rh->AccessTH1("nstrip", 40, 0, 20);
    auto _hns = _rh->AccessTH1("ns", 20, 0.1, 20.1);
    _hnstrip->Fill(strips.size());

    if (!strips.empty())
    {
      if (strips.size()<9)
	_hns->Fill(strips.size());
        std::sort(
            strips.begin(),
            strips.end(),
            [](const std::shared_ptr<StripHit> &a,
               const std::shared_ptr<StripHit>& b)
            {
                if (a->thr!=b->thr)
                    return a->thr<b->thr;

                return a->strip<b->strip;
            });

        auto const& s =
            strips.front();
        
        auto _hczs = _rh->AccessTH1("czs", 200, -50, 150);
        auto _hcxy = _rh->AccessTH2("cxy", 80,0.,160.,30,0.,60.);
        auto _hcpos = _rh->AccessTH2("cpos", 80,0.,160.,60,0.,60.);
        auto _hcstrip = _rh->AccessTH1("cstrip", 50, 0, 50);
        _hcxy->Fill(
            s->yloc,
            s->xloc);

        _hcstrip->Fill(
            s->strip);

        _hczs->Fill(
            s->zs);

        _hcpos->Fill(s->zs,s->strip);
    }

    if (found && flow && fhigh)
    {
        _nfound++;
	_nstrip+=strips.size();
        _hstat->Fill(11);
	if (strips.size()>0)  _hstat->Fill(21);
    }
    auto vclusters=this->clusteriserHits(strips);
    for (size_t i = 0; i < vclusters.size(); ++i) {
      

      std::sort(vclusters[i].begin(), vclusters[i].end(),
		[](std::shared_ptr<StripHit> a, std::shared_ptr<StripHit> b) { return a->thr < b->thr; });
      auto c=std::make_shared<FebCluster>(vclusters[i]);
      _clusters.push_back(c);
      /*
      std::cout << "Cluster " << i + 1 << " : Tmin " << c->t()<<" strips ";
      for (auto p : c->hits()) {
	std::cout << p->strip << " ";
      }
        
      std::cout << std::endl;
      std::cout<<c->x()<<" "<<c->y()<<" "<<c->zs()<<std::endl;
      */
      
    }
    std::sort(_clusters.begin(), _clusters.end(),
	      [](std::shared_ptr<FebCluster> a, std::shared_ptr<FebCluster> b) { return a->t() < b->t(); });

    auto _hncl = _rh->AccessTH1("ncl", 20, 0., 20);
    _hncl->Fill(_clusters.size());
    if (_clusters.size())
      {
	auto c= _clusters[0];
	auto _h1thr = _rh->AccessTH1("h1thr",2000,-2000.,0.);
	auto _hc1zs = _rh->AccessTH1("c1zs", 200, -50, 150);
        auto _hc1xy = _rh->AccessTH2("c1xy", 80,0.,160.,30,0.,60.);
        auto _hc1pos = _rh->AccessTH2("c1pos", 80,0.,160.,60,0.,60.);
        auto _hc1strip = _rh->AccessTH1("c1strip", 50, 0, 50);
	_hc1zs->Fill(c->zs());
	_hc1xy->Fill(c->y(),c->x());
	_hc1pos->Fill(c->zs(),c->strip());
	_hc1strip->Fill(c->strip());
	_h1thr->Fill(c->t());
	
      }
    /*
    for (size_t i = 0; i < _clusters.size(); ++i) {
      auto c= _clusters[i];
      std::cout << "Cluster " << i + 1 << " : Tmin " << c->t()<<" strips ";
      for (auto p : c->hits()) {
	std::cout << p->strip << " ";
      }
        
      std::cout << std::endl;
      std::cout<<c->x()<<" "<<c->y()<<" "<<c->zs()<<std::endl;
    }
    getchar();
    */
}

////////////////////////////////////////////////////////////////
/// end
////////////////////////////////////////////////////////////////

void FebProcessor::end(uint32_t)
{
  auto _hns = _rh->AccessTH1("ns", 20, 0.1, 20.1);
  double mul=_hns->GetMean();
    std::string fname =
        "feb_" + std::to_string(_run) + ".root";

    std::cout
        << "Run "
        << _run
        << std::endl;
    _rh->writeHistograms(fname);
    auto neff=3560.*_nread/(3560+126);
    std::cout
        << "Processed "
        << _nread
        << " events  found="
        << _nfound
	<<" Efficiency: " 
	<< (_nfound * 100.0 / neff) << "%  Multiplicity " << 1.*_nstrip/_nfound <<" ns<9 "<<mul<<"\n";

        
}



#include <vector>
#include <algorithm>


std::vector<std::vector<std::shared_ptr<StripHit>> > FebProcessor::clusteriserHits(const std::vector<std::shared_ptr<StripHit>>& hits) {
    // Trier par numéro de strip
    std::vector<std::shared_ptr<StripHit>> sortedHits = hits;
    std::sort(sortedHits.begin(), sortedHits.end(),
              [](std::shared_ptr<StripHit> a, std::shared_ptr<StripHit> b) { return a->strip < b->strip; });
    
    std::vector<std::vector<std::shared_ptr<StripHit>>> clusters;
    
    if (sortedHits.empty()) return clusters;
    
    std::vector<std::shared_ptr<StripHit>> clusterCourant;
    clusterCourant.push_back(sortedHits[0]);
    
    for (size_t i = 1; i < sortedHits.size(); ++i) {
        int ecart = sortedHits[i]->strip - sortedHits[i-1]->strip;
        
        if (ecart <= 2) {
            clusterCourant.push_back(sortedHits[i]);
        } else {
            clusters.push_back(clusterCourant);
            clusterCourant.clear();
            clusterCourant.push_back(sortedHits[i]);
        }
    }
    
    clusters.push_back(clusterCourant);
    return clusters;
}
////////////////////////////////////////////////////////////////
/// plugin factory
////////////////////////////////////////////////////////////////

extern "C"
lmana::rbProcessor* createProcessor()
{
    return new lmana::FebProcessor();
}
