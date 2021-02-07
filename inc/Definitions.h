
#ifndef STEWARDESS_DEFINITIONS_H_
#define STEWARDESS_DEFINITIONS_H_

#include <chrono>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "logtastic.h"


#define STEWARDESS_VERSION_STRING "0.1.0"


namespace Stewardess
{

  // The default time for the internal tick rate of the server
  const timeval defaultTickTime = { 2, 0 };


  class Connection;
  class Payload;
  class Buffer;
  class ThreadInfo;

  // Serialization structures
  typedef std::queue< Buffer* > BufferQueue;
  typedef std::queue< Payload* > PayloadQueue;
  typedef std::queue< const char* > ErrorQueue;

  // Prefered time stamp data type
  typedef std::chrono::time_point<std::chrono::system_clock> TimeStamp;
  typedef std::chrono::milliseconds Milliseconds;
  typedef std::chrono::seconds Seconds;

  // Really big unique id type
  typedef long long unsigned int UniqueID;

  // Common arry-like structures
  typedef std::map< size_t, Connection* > ConnectionMap;
  typedef std::list< Connection* > ConnectionList;
  typedef std::vector< ThreadInfo* > ThreadVector;

  // Short hands for mutex locks
  typedef std::unique_lock<std::mutex> UniqueLock;
  typedef std::lock_guard<std::mutex> GuardLock;


  ////////////////////////////////////////////////////////////////////////////////
  // Connection event enumeration

  enum class ConnectionEvent { Connect, Disconnect, DisconnectError, Timeout, SerializationError };

  enum class ServerEvent { Shutdown };
}

#endif // STEWARDESS_DEFINITIONS_H_

