
#ifndef MANAGER_DATA_H_
#define MANAGER_DATA_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <iostream>
#include <cstring>
#include <thread>


// Forward declare the manager class so we can hold a pointer to our owner
class Manager;
class MessageBuilderBase;

struct ManagerData
{
  // Pointer to owner
  Manager* owner;

  // The server pointer
  ServerState* server;


  // The listener event base. Platform for the listener event to be launched from
  event_base* eventBase;

  // Pointer to the listener event
  evconnlistener* listener;

  // Pointer to the signal event
  event* signalEvent;


  // The socket we're listening on
  evutil_socket_t socket;

  // The address of the socket we're listening to
  sockaddr_in socketAddress;


  // Time out time for read/write attempts
  timeval timeout;

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

