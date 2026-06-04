#include "rbReader.hh"
#include "rbProcessor.hh"
#include "rbRun.hh"
#include "rbEvent.hh"

#include <fstream>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <zstd.h>

#include <json/json.h>

namespace lmana
{

#pragma pack(push,1)

struct RecordHeader
{
    uint32_t type;
    uint32_t number;
    uint32_t length;
    uint64_t timestamp;
};

struct BlockHeader
{
    uint32_t blockType;
    uint32_t eventNumber;
    uint32_t blockId;
    uint32_t compressedSize;
};

#pragma pack(pop)

static std::vector<uint8_t>
decompressBuffer(const void* src,size_t size)
{
    unsigned long long outSize=
        ZSTD_getFrameContentSize(src,size);

    if (outSize==ZSTD_CONTENTSIZE_ERROR)
        throw std::runtime_error("Invalid zstd frame");

    std::vector<uint8_t> out(outSize);

    size_t ret=
        ZSTD_decompress(
            out.data(),
            outSize,
            src,
            size);

    if (ZSTD_isError(ret))
        throw std::runtime_error(
            ZSTD_getErrorName(ret));

    return out;
}

void rbReader::read(const std::string& fileName,
                    rbProcessor* processor)
{
    std::ifstream in(fileName,std::ios::binary);

    if (!in)
        throw std::runtime_error("Cannot open file");

    RecordHeader hdr;

    while (in.read(
           reinterpret_cast<char*>(&hdr),
           sizeof(hdr)))
    {
        if (hdr.type==0)
        {
            std::vector<char> compressed(hdr.length);

            in.read(compressed.data(),hdr.length);
	    std::vector<uint8_t> raw;
	    try
	      {
		raw =
		  decompressBuffer(
				   compressed.data(),
				   compressed.size());
	      }
	    catch(const std::exception& e)
	      {
		std::cerr
		  << "Exception : "
		  << e.what()
		  << std::endl;
		break;
	      }
	    /*
            auto raw=
                decompressBuffer(
                    compressed.data(),
                    compressed.size());
	    */
            rbRun run;

            run.runNumber=hdr.number;
            run.timestamp=hdr.timestamp;
            run.isJson=false;

            size_t n=
                raw.size()/sizeof(uint32_t);

            run.rawHeader.resize(n);

            memcpy(run.rawHeader.data(),
                   raw.data(),
                   raw.size());

            processor->processRunHeader(&run);
        }
        else if (hdr.type==2)
        {
            std::vector<char> compressed(hdr.length);

            in.read(compressed.data(),hdr.length);
	    std::vector<uint8_t> raw;
	    try
	      {
		raw =
		  decompressBuffer(
				   compressed.data(),
				   compressed.size());
	      }
	    catch(const std::exception& e)
	      {
		std::cerr
		  << "Exception : "
		  << e.what()
		  << std::endl;
		break;
	      }

            //auto raw=
            //    decompressBuffer(
            //        compressed.data(),
            //        compressed.size());

            std::string jsonText(
                reinterpret_cast<char*>(raw.data()),
                raw.size());

            Json::CharReaderBuilder builder;

            Json::Value root;
            std::string errs;

            std::stringstream ss(jsonText);

            if (!Json::parseFromStream(
                    builder,
                    ss,
                    &root,
                    &errs))
                throw std::runtime_error(errs);

            rbRun run;

            run.runNumber=hdr.number;
            run.timestamp=hdr.timestamp;
            run.isJson=true;
            run.jsonHeader=root;

            processor->processRunHeader(&run);
        }
        else if (hdr.type==1)
        {
            rbEvent evt;

            evt.eventNumber=hdr.number;
            evt.timestamp=hdr.timestamp;

            evt.blocks.resize(hdr.length);

            for (uint32_t i=0;i<hdr.length;i++)
            {
                BlockHeader bh;

                in.read(
                    reinterpret_cast<char*>(&bh),
                    sizeof(bh));

                std::vector<char>
                    compressed(
                        bh.compressedSize);

                in.read(
                    compressed.data(),
                    bh.compressedSize);
	    std::vector<uint8_t> raw;
	    try
	      {
		raw =
		  decompressBuffer(
				   compressed.data(),
				   compressed.size());
	      }
	    catch(const std::exception& e)
	      {
		std::cerr
		  << "Exception : "
		  << e.what()
		  << std::endl;
		break;
	      }

	    //auto raw=
	    ///       decompressBuffer(
            //            compressed.data(),
            //            compressed.size());

                size_t nw=
                    raw.size()/sizeof(uint64_t);

                evt.blocks[i].resize(nw);

                memcpy(
                    evt.blocks[i].data(),
                    raw.data(),
                    raw.size());
            }

            processor->processEvent(&evt);
        }
    }
}

}
