#pragma once
#include <zmq.hpp>
#include "pmBuffer.hh"

namespace pm
{
  /**
     \class pmSender
     \brief The default object to send data to the event builder
     \details It provides all the mechanism to push data (PUSH/PULL) mechanism to the event builder
        - it allocates a ZMQ_PUSH socket
	- it creates a 512 kBytes pm::buffer
    \author    Laurent Mirabito
    \version   1.0
    \date      January 2019
    \copyright GNU Public License.
  */
class pmSender
{
public:
  /**
     \brief Constructor
     \param c , the ZMQ context
     \param det , the detector Id of the allocated buffer
     \param det , the data source Id of the allocated buffer
   */
  pmSender( zmq::context_t* c, uint32_t det,uint32_t dif);

  /**
     \brief connect the socket to its client (Event Builder)
     \param dest, client address (ex  "tcp://lyoxxx:5555")
   */
  void connect(std::string dest);

  /**
     \brief Send the buffer to the socket
     \param bx ,  bunch crossing id
     \param gtc , event or window id
     \param len , actual len of the buffer
   */
  void publish(uint64_t bx, uint32_t gtc,uint32_t len);

  /**
     \brief Send an ID message on the socket in order to register the data source in the collector process
   */
  void collectorRegister();

  /**
     \brief access to the buffer payload
     \return Pointer to the payload
   */
  char* payload();

  /**
     \brief Switch compression (zlib)
     \param t , True of False
   */
  inline void setCompress(bool t){_compress=t;}

  /**
     \brief Compressions status
     \return True if compression active
   */
  inline bool isCompress() {return _compress;}

  /**
     \brief Access to pm::buffer
     \return pointer to the pm::buffer
   */
  inline pm::buffer* buffer(){return _buffer;}

  /**
     \brief Detector ID
   */
  inline uint32_t detectorId(){return _detId;}

    /**
     \brief Data source ID
   */
  inline uint32_t sourceId(){return _sourceId;}
  /**
     \brief Discover builder application to connact automatically
     \param session the name of the session
     \param appname the name of the Event Builder (zmMeger) application
     \param portname the name of parameter of the appilcation specifying the listening port
   */
  void autoDiscover(std::string session,std::string appname,std::string portname);
private:
  zmq::context_t* _context;
  uint32_t _detId,_sourceId;
  std::vector<zmq::socket_t *> _vSender;
  std::string _header;
  pm::buffer* _buffer;
  bool _compress;
};
};
