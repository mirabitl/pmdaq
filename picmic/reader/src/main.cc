#include "rbReader.hh"
#include "rbProcessor.hh"
#include "TApplication.h"
#include "TROOT.h"

#include <dlfcn.h>
#include <iostream>
#include "DCHistogramHandler.hh"
using namespace lmana;

int main(int argc,char** argv)
{

    if (argc<3)
    {
        std::cerr
            << argv[0]
            << " file.bin plugin.so"
            << std::endl;

        return 1;
    }
    DCHistogramHandler* rh=DCHistogramHandler::instance();
  

    void* handle=
        dlopen(argv[2],RTLD_NOW);

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


    proc->init();

    rbReader reader;

    reader.read(argv[1],proc);

    proc->end();

    delete proc;

    dlclose(handle);

    return 0;
}
