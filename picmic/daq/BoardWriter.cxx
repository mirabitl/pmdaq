#include "BoardWriter.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "utils.hh"

#define RUN_HEADER_START 0x5b
#define RUN_HEADER_END 0x5d
#define READOUT_START 0x28
#define READOUT_END 0x29


BoardWriter::BoardWriter(std::string dire, std::string location) : _directory(dire), _location(location), _run(0), _idx(0), _fdOut(-1), _totalSize(0), _event(0), _useShm(false), _shmDirectory("") {}
void BoardWriter::newRun(uint32_t run, bool resetEventNumber)
{
  _run = run;

  if (!_useShm)
  {
    std::stringstream filename("");
    char dateStr[64];

    time_t tm = time(NULL);
    strftime(dateStr, 20, "SMM_%d%m%y_%H%M%S", localtime(&tm));
    filename << _directory << "/" << _location << "_R" << run << "_" << dateStr << ".dat";
    _fdOut = ::open(filename.str().c_str(), O_CREAT | O_RDWR | O_NONBLOCK, S_IRWXU);
    if (_fdOut < 0)
    {
      perror("No way to store to file :");
      // std::cout<<" No way to store to file"<<std::endl;
      return;
    }

    char mode[] = "0744";
    int im;
    im = strtol(mode, 0, 8);

    int ier = chmod(filename.str().c_str(), im);
  }
  if (resetEventNumber)
    _event = 0;
}
void BoardWriter::endRun()
{

  if (_fdOut > 0 && !_useShm)
  {
    ::close(_fdOut);
    _fdOut = -1;
  }
}
uint32_t BoardWriter::totalSize() { return _totalSize; }
uint32_t BoardWriter::eventNumber() { return _event; }
uint32_t BoardWriter::runNumber() { return _run; }
void BoardWriter::setIds(uint32_t did,uint32_t sid)
{_detectorId=did;_sourceId=sid;}

void BoardWriter::writeRunHeader(std::vector<uint32_t> header)
{
  char rhstart = RUN_HEADER_START;
  char rhend = RUN_HEADER_END;
  if (_fdOut > 0 && header.size() < 256)
  {
    int ier = write(_fdOut, &rhstart, 1);
    uint32_t ibuf[256];
    for (int i = 0; i < header.size(); i++)
      ibuf[i] = header[i];
    std::copy(header.begin(), header.end(), ibuf);
    ier = write(_fdOut, &_run, sizeof(uint32_t));
    int nb = header.size();
    ier = write(_fdOut, &nb, sizeof(uint32_t));
    ier = write(_fdOut, ibuf, nb * sizeof(uint32_t));
    ier = write(_fdOut, &rhend, 1);
    _totalSize += (nb + 1) * sizeof(uint32_t) + 2;
  }
}
void BoardWriter::newEvent()
{
  _event++;
  _idx = 0;
  _eventSize = 0;
  _buffer[0] = READOUT_START;
  uint32_t *ibuf = (uint32_t *)&_buffer[1];
  ibuf[0] = _event;
  //_ibuf[1] will contain the number of 32 bits integer word to read
  _idx = 9;
}
uint32_t BoardWriter::appendEventData(std::vector<uint32_t> val)
{
  uint32_t nv = val.size();
  _eventSize += nv;
  uint32_t *ibuf = (uint32_t *)&_buffer[_idx];
  std::copy(val.begin(), val.end(), ibuf);
  _idx += nv * sizeof(uint32_t);
  return _idx;
}
uint32_t BoardWriter::writeEvent()
{
  if (_fdOut > 0)
  {
    uint32_t *ibuf = (uint32_t *)&_buffer[1];
    ibuf[1] = _eventSize;
    _buffer[_idx] = READOUT_END;
    ++_idx;
    int ier = write(_fdOut, _buffer, _idx);
    _totalSize += _idx;
    _event++;

    if (_totalSize > 1900 * 1024 * 1024)
    {
      ::close(_fdOut);
      _totalSize = 0;
      this->newRun(_run, false);
    }
  }
  if (_useShm)
  {
    uint32_t *ibuf = (uint32_t *)&_buffer[1];
    ibuf[1] = _eventSize;
    _buffer[_idx] = READOUT_END;
    ++_idx;
    utils::store(_detectorId,_sourceId,_event,_event, _buffer,_idx, _shmDirectory);
    _totalSize += _idx;


    
  }
  return _idx;
}

std::string BoardWriter::file_name() { return _file_name; }
bool BoardWriter::useShm() { return _useShm; }
void BoardWriter::setShmDirectory(std::string shd)
{
  _shmDirectory = shd;
  _useShm = true;
}
