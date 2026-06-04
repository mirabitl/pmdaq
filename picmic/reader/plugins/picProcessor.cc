#include "rbProcessor.hh"
#include "rbRun.hh"
#include "rbEvent.hh"
#include "DCHistogramHandler.hh"
#include <json/json.h>

#include <TFile.h>
#include <TH1F.h>
#include <TF1.h>
#include <TROOT.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>
#include <map>
#include <array>
#include <vector>
#include <cmath>

static TCanvas* c1;
namespace
{
class BitField
{
public:
    explicit BitField(uint32_t v) : _value(v) {}

    inline uint32_t bit(int b) const
    {
        return (_value >> b) & 0x1;
    }

    inline uint32_t range(int msb,int lsb) const
    {
        uint32_t mask =
            (1u << (msb-lsb+1)) - 1;

        return (_value >> lsb) & mask;
    }

private:
    uint32_t _value;
};
}

namespace lmana
{

class picProcessor : public rbProcessor
{
public:

    picProcessor()
    {
        _histos.fill(nullptr);
	_rh=DCHistogramHandler::instance();
	_nevt=0;
	_counts.fill(0);
    }

    virtual ~picProcessor() = default;

    void init(uint32_t run=0) override
    {
        std::cout
            << "picProcessor::init run "
            << run
            << std::endl;
	
        
    }

    void end(uint32_t run=0) override
    {
        std::cout
            << "picProcessor::end run "
            << _run
            << std::endl;



        std::stringstream sr;
	sr << "histo" <<_run << "_0.root";
	_rh->writeHistograms(sr.str());
        std::stringstream srr;
	srr << "results" <<_run << ".json";
	std::ofstream file(srr.str());

	Json::StreamWriterBuilder builder;
	builder["indentation"] = "  ";

	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(_results, &file);

	file.close();
    }

    void loadParameters(Json::Value params) override
    {
        _params = params;
    }

    void processRunHeader(rbRun* r) override
    {
        std::cout
            << "Run "
            << r->runNumber
            << std::endl;
	_run=r->runNumber;
	if (!_results.isMember("run"))
	  _results["run"]=_run;
	if (_counts[0]!=0 && _vin!=0)
	  {
	    
	    float eff=_counts[13]*100./_counts[0];
	    printf("%d %d Eff-> %d %f\n",_counts[0],_counts[13],_vin,eff);
	    //getchar();
	    _eff.insert(std::pair<uint32_t,double>(_vin,eff));
	    _results["stat"][Form("%d",_vin)]["ntot"]=_counts[0];
	    _results["stat"][Form("%d",_vin)]["nseen"]=_counts[13];
	    _results["stat"][Form("%d",_vin)]["eff"]=eff;
	    for (int ch=0;ch<64;ch++)
	      {
		auto hd=_rh->GetTH1(Form("diff%d_%d",ch,_vin));
		if (hd!=NULL)
		  {
		    _results["channels"][Form("CH%d",ch)][Form("%d",_vin)]["mean"]=hd->GetMean();
		    _results["channels"][Form("CH%d",ch)][Form("%d",_vin)]["rms"]=hd->GetRMS()*1000.;
		    _results["channels"][Form("CH%d",ch)][Form("%d",_vin)]["d_mean"]=hd->GetMeanError();
		    _results["channels"][Form("CH%d",ch)][Form("%d",_vin)]["d_rms"]=hd->GetRMSError()*1000.;

		  }
	      }

	  }
	_counts.fill(0);
        if (r->isJson)
        {
            std::cout
                << r->jsonHeader.toStyledString()
                << std::endl;
	    _results["def"]=r->jsonHeader;
        }
	else
	  {
	    //for (auto x :r->rawHeader)
	    //  std::cout<<x<<std::endl;
	    _vin=r->rawHeader[2];
	    std::cout<<"injection "<<_vin<<std::endl;
	    _nevt=0;
	    //getchar();
	    for (auto h:_histos)
	      if (h!=nullptr)
		{
		c1->cd();
		h->Draw();
		c1->Draw();
		c1->Update();
		getchar();
		}
	  }
	
    }

    void processEvent(rbEvent* e) override
    {
        std::array<std::vector<double>,64> ts;
        std::array<bool,64> first;
        first.fill(true);
	_nevt++;
        bool newwin = false;
	float last_t0=0;
	uint32_t nwin=0;
	std::array<uint32_t,64> nfound;
	nfound.fill(0);
        for(size_t p=0;
            p<e->blocks.size();
            p++)
        {
            for(auto raw64 : e->blocks[p])
            {
                uint32_t w =
                    static_cast<uint32_t>(raw64);

                BitField word(w);

                if(word.bit(31))
                {
                    uint32_t bxcount =
                        word.range(27,15);

                    uint32_t coarsecount =
                        word.range(14,2);

                    (void)bxcount;
                    (void)coarsecount;

                    continue;
                }

                int ch =
                    word.range(30,27)
                    + 16*p;

                if(ch<0 || ch>=64)
                    continue;

                if(first[ch])
                {
                    first[ch]=false;
                    continue;
                }

                uint32_t coarse =
                    word.range(25,13);

                uint32_t fine =
                    word.range(12,0);

                double t =
                    ((coarse<<13)|fine)
                    *3.0523e-3;

                if(ch==0)
                {
                    ts[0].push_back(t);
		    last_t0=t;
                    newwin=true;
		    nwin++;
		    nfound[0]++;
		    _counts[0]++;
                }
                else
                {
                    //
                    // Reproduction exacte du Python:
                    // un seul hit par fenêtre
                    //
                    if(newwin)
                    {
                        ts[ch].push_back(t);
			auto hdif=_rh->AccessTH1(Form("diff%d_%d",ch,_vin),200,t-last_t0-0.8,t-last_t0+0.8);
			hdif->Fill(t-last_t0);
			nfound[ch]++;
			_counts[ch]++;
                        newwin=false;
			
                    }
                }
            }
        }

        for(auto const& m : _liroc2ptdc)
        {
            int lch = m.first;
            int pch = m.second;

            (void)lch;

            if(ts[0].empty())
                continue;

            if(ts[pch].size()!=ts[0].size())
                continue;

            if(ts[pch].size()<25)
                continue;

            std::vector<double> td;
            td.reserve(ts[pch].size());

            for(size_t i=0;
                i<ts[pch].size();
                i++)
            {
                td.push_back(
                    ts[pch][i]
                    - ts[0][i]);
            }

            double mean=0.;
            double rms=0.;

            for(auto v : td)
                mean+=v;

            mean/=td.size();

            for(auto v : td)
                rms+=(v-mean)*(v-mean);

            rms=std::sqrt(rms/td.size());

                double xmin =
                    mean-10.*rms;

                double xmax =
                    mean+10.*rms;

                if(xmin==xmax)
                {
                    xmin-=0.5;
                    xmax+=0.5;
                }
		if (xmax-xmin>1.5)
		  {
		    xmin=mean-0.75;
		    xmax=mean+0.75;
		    
		  }

	    auto hd=_rh->AccessTH1(Form("chanP%dL%d_%d",pch,lch,_vin),250,xmin,xmax);
	    for(auto v : td)
	      hd->Fill(v);
        }
	if (false)
	  {
	std::cout<<"Event "<<_nevt<<" Inj "<< _vin<< "Number of windows "<<nwin<<std::endl;
	if (nwin>0)
	  {
	  for (const auto& x:nfound)
	    std::cout<<x<<" ";
	  std::cout<<std::endl;
	  getchar();
	  }
	  }
    }

private:
  Json::Value _results;
    Json::Value _params;
  
    TFile* _rootFile{nullptr};

  std::array<TH1F*,64> _histos;
  std::array<uint32_t,64> _counts;
  std::map<uint32_t,double> _eff;
  uint32_t _vin,_run,_nevt;
  static const std::map<int,int> _liroc2ptdc;
  DCHistogramHandler* _rh;
};

const std::map<int,int>
picProcessor::_liroc2ptdc =
{
 {62,3},{58,4},{63,5},{54,6},{60,7},{61,8},
 {56,9},{44,10},{50,11},{59,12},{46,13},{57,14},
 {52,15},{48,16},{53,17},{51,18},{42,19},{49,20},
 {40,21},{47,22},{38,23},{45,24},{36,25},{43,26},
 {34,27},{41,28},{32,29},{39,30},{30,31},{37,32},
 {28,33},{35,34},{26,35},{33,36},{24,37},{31,38},
 {22,39},{29,40},{20,41},{27,42},{18,43},{25,44},
 {16,45},{23,46},{21,47},{12,48},{17,49},{5,50},
 {19,51},{6,52},{15,53},{4,54},{13,55},{14,56},
 {11,57},{10,58},{2,59},{3,60},{7,61},{8,62},
 {1,63}
};

}

extern "C"
lmana::rbProcessor*
createProcessor()
{
    return new lmana::picProcessor();
}
