
#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <chrono>
#include <mutex>
#include <thread>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <iostream>


class Connection;
class Handler;


// Prefered time stamp data type
typedef std::time_t TimeStamp;

// Common arry-like structures
typedef std::map< size_t, Connection* > ConnectionMap;
typedef std::list< Handler* > ThreadList;

// Short hands for mutex locks
typedef std::unique_lock<std::mutex> UniqueLock;
typedef std::lock_guard<std::mutex> GuardLock;


////////////////////////////////////////////////////////////////////////////////
// Connection event enumeration

enum class ConnectionEvent { Connect, Disconnect, Timeout, Error, ReadError, WriteError };

enum class ServerEvent { Shutdown };

#endif // DEFINITIONS_H_

