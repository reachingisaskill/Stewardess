
#ifndef STEWARDESS_DEFINITIONS_H_
#define STEWARDESS_DEFINITIONS_H_

#include <chrono>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>

#include "logtastic.h"


#define STEWARDESS_VERSION_STRING "0.1.0"


namespace Stewardess
{

  // The default time for the internal tick rate of the server
  const timeval defaultTickTime = { 2, 0 };
  // Start an timer event immediately with this timer
  const timeval immediately = { 0, 0 };


  // Forward declare some classes
  class Connection;
  class Payload;
  class Buffer;
  class ThreadInfo;
  class TimerData;


  // Serialization structures
  typedef std::queue< Buffer* > BufferQueue;
  typedef std::queue< Payload* > PayloadQueue;
  typedef std::queue< const char* > ErrorQueue;

  // Prefered time stamp data type
  typedef std::chrono::time_point<std::chrono::system_clock> TimeStamp;
  typedef std::chrono::milliseconds Milliseconds;
  typedef std::chrono::seconds Seconds;

  // Unique identifier types
  typedef std::intptr_t ConnectionID;
  typedef int64_t UniqueID;

  // Common arry-like structures
  typedef std::map< ConnectionID, Connection* > ConnectionMap;
  typedef std::vector< ThreadInfo* > ThreadVector;
  typedef std::unordered_map< UniqueID, TimerData* > TimerMap;

  // Short hands for mutex locks
  typedef std::unique_lock<std::mutex> UniqueLock;
  typedef std::lock_guard<std::mutex> GuardLock;


  ////////////////////////////////////////////////////////////////////////////////
  // Connection event enumeration

  enum class ConnectionEvent { Connect, Disconnect, DisconnectError, Timeout, SerializationError };

  enum class ServerEvent { Shutdown, ListenerError, RequestConnectFail };


  ////////////////////////////////////////////////////////////////////////////////
  // Useful template functions
  template< typename DURATION >
  timeval convertToTimeval( DURATION duration )
  {
    auto seconds = std::chrono::duration_cast< std::chrono::seconds >( duration );
    auto micros = std::chrono::duration_cast< std::chrono::microseconds >( duration-seconds );

    return { (time_t)seconds.count(), (long int)micros.count() };
  }

}

#endif // STEWARDESS_DEFINITIONS_H_

