
#ifndef MANAGER_DATA_H_
#define MANAGER_DATA_H_

#include "Definitions.h"
#include "LibeventIncludes.h"

#include <iostream>
#include <cstring>
#include <thread>


// Forward declare the manager class so we can hold a pointer to our owner
class CallbackInterface;
class ManagerBase;

struct ManagerData
{
  // Pointer to owner
  ManagerBase* owner;

  // The server pointer
  CallbackInterface* server;

  // Map of all the connections currently with the process
  ConnectionMap connections;
  std::mutex connectionMapMutex;

  // Queue of the connection that have been signalled to close
  ConnectionQueue closedConnections;
  

  // The listener event base. Platform for the listener event to be launched from
  event_base* eventBase;

  // Pointer to the listener event
  evconnlistener* listener;

  // Pointer to the signal event
  event* signalEvent;

  // Pointer to the signal event
  event* deathEvent;


  // The port number we want
  int portNumber;

  // The socket we're listening on
  evutil_socket_t socket;

  // The address of the socket we're listening to
  sockaddr_in socketAddress;

  // Address of remove server we're trying to connect to
  std::string clientAddress;


  // Modify the internal tick time by this factor
  float tickTimeModifier;

  // Time out time for read/write attempts
  timeval readTimeout;
  timeval writeTimeout;

  // Time out time for forcing the server to shutdown
  timeval deathTime;

  // Set whether the manager can signal connection closes on shutdown
  bool connectionCloseOnShutdown;

  // Default size for the read/write buffers
  size_t bufferSize;


  // Number of parallel threads to handle connection events
  unsigned numThreads;

  // Listener thread
  ThreadVector threads;

  // The last thread that used
  size_t nextThread;

};


#endif // MANAGER_DATA_H_

