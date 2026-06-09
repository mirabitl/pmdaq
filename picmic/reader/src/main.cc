#include "rbReader.hh"
#include "rbProcessor.hh"
#include "TApplication.h"
#include "TROOT.h"

#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include "DCHistogramHandler.hh"
#include <json/json.h>

using namespace lmana;

int main(int argc,char** argv)
{

    if (argc<2)
    {
        std::cerr
            << argv[0]
            << " params.json"
            << std::endl;

        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file)
      {
        std::cerr << "reader file '" << argv[1] << "'\n";
        return -1;
      }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::Value root;
    std::string errors;

    if (!Json::parseFromStream(builder, file, &root, &errors))
    {
        std::cerr << "rbreader: JSON parse error: " << errors << '\n';
        return -1;
    }

    if (!root.isMember("plugin"))
    {
      std::cerr << "No plugin " << root << "'\n";
      exit(-1);
    }
    if (!root.isMember("file"))
    {
      std::cerr << "No file " << root << "'\n";
      exit(-1);
    }
    if (!root.isMember("parameters"))
    {
      std::cerr << "No parameters " << root << "'\n";
      exit(-1);
    }


    
    DCHistogramHandler* rh=DCHistogramHandler::instance();
  

    void* handle=
      dlopen(root["plugin"].asString().c_str(),RTLD_NOW);

    if (!handle)
    {
        std::cerr
            << dlerror()
            << std::endl;
        return 1;
    }

    auto create=
        reinterpret_cast<
            rbProcessor*(*)()>(
                dlsym(
                    handle,
                    "createProcessor"));

    rbProcessor* proc=create();

    proc->loadParameters(root["parameters"]);
    proc->init();

    rbReader reader;

    reader.read(root["file"].asString(),proc);

    proc->end();

    delete proc;

    dlclose(handle);

    return 0;
}
