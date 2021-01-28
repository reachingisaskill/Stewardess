
#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <chrono>
#include <map>
#include <string>


class Connection;


//typedef std::chrono::time_point<std::chrono::system_clock> TimeStamp;
typedef std::time_t TimeStamp;

typedef std::map< size_t, Connection* > ConnectionMap;

#endif // DEFINITIONS_H_

