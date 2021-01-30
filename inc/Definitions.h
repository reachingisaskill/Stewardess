
#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <chrono>
#include <mutex>
#include <thread>
#include <list>
#include <queue>
#include <vector>
#include <map>
#include <string>
#include <iostream>


class Connection;
class Handler;
class Payload;
class BufferData;
class Buffer;

// Serialization structures
typedef std::queue< Buffer* > BufferQueue;
typedef std::queue< Payload* > PayloadQueue;
typedef std::queue< const char* > ErrorQueue;

// Prefered time stamp data type
typedef std::time_t TimeStamp;

// Common arry-like structures
typedef std::map< size_t, Connection* > ConnectionMap;
typedef std::vector< Handler* > ThreadVector;

// Short hands for mutex locks
typedef std::unique_lock<std::mutex> UniqueLock;
typedef std::lock_guard<std::mutex> GuardLock;


////////////////////////////////////////////////////////////////////////////////
// Connection event enumeration

enum class ConnectionEvent { Connect, Disconnect, Timeout, Error, ReadError, WriteError };

enum class ServerEvent { Shutdown };

#endif // DEFINITIONS_H_

