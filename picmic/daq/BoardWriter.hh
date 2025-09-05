/* File : example.h */

#include <vector>
#include <string>
#include <stdint.h>
#define MAX_EVENT_SIZE 0x400000

class BoardWriter
{
public:
  //BoardWriter(std::string dire,std::string location,std::string condition);
  BoardWriter(std::string dire,std::string location);
  void newRun(uint32_t runid,bool resetEventNumber=true);
  void endRun();
  void writeRunHeader(std::vector<uint32_t> header);
  uint32_t eventNumber();
  uint32_t runNumber();
  uint32_t totalSize();
  void newEvent();
  uint32_t appendEventData(std::vector<uint32_t> val);
  uint32_t writeEvent();
  std::string file_name();
  bool useShm();
  void setShmDirectory(std::string shmd); 
  void setIds(uint32_t did,uint32_t sid);
  inline uint32_t detectorId(){return _detectorId;}
  inline uint32_t sourceId(){return _sourceId;}
private:
  std::string _directory;
  std::string _location;
  std::string _file_name;
  int32_t _fdOut;
  uint32_t _run,_event,_idx,_totalSize,_eventSize;
  unsigned char _buffer[MAX_EVENT_SIZE];
  bool _useShm;
  std::string _shmDirectory;
  uint32_t _detectorId,_sourceId;
};
  
