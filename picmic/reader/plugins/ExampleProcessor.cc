#include "rbProcessor.hh"
#include "rbRun.hh"
#include "rbEvent.hh"

#include <iostream>

using namespace lmana;

class ExampleProcessor : public rbProcessor
{
public:

    void init(uint32_t run) override
    {
        std::cout
            << "Init run "
            << run
            << std::endl;
    }

    void end(uint32_t run) override
    {
        std::cout
            << "End run "
            << run
            << std::endl;
    }

    void loadParameters(Json::Value p) override
    {
        params_=p;
    }

    void processRunHeader(rbRun* r) override
    {
        std::cout
            << "Run "
            << r->runNumber
            << std::endl;

        if (r->isJson)
        {
            std::cout
                << r->jsonHeader.toStyledString()
                << std::endl;
        }
        else
        {
            std::cout
                << "Header size="
                << r->rawHeader.size()
                << std::endl;
        }
    }

    void processEvent(rbEvent* e) override
    {
        std::cout
            << "Event "
            << e->eventNumber
            << " blocks="
            << e->blocks.size()
            << std::endl;

        for(size_t i=0;i<e->blocks.size();i++)
        {
            std::cout
                << " block "
                << i
                << " words="
                << e->blocks[i].size()
                << std::endl;
        }
    }

private:
    Json::Value params_;
};

extern "C"
rbProcessor* createProcessor()
{
    return new ExampleProcessor();
}
