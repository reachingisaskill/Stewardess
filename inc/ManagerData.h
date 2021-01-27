
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

struct ManagerData
{
  // Pointer to owner
  Manager* owner;

  // Listener thread
  std::thread listenerThread;

  // Port to listen on
  int portNumber;

  // The socket we're listening on
  evutil_socket_t socket;

  // The address of the socket we're listening to
  sockaddr_in socketAddress;

  // Time out time for read/write attempts
  timeval timeout;



  // Basic setup
  ManagerData( Manager*, int );

  // Start the listener event
  void dispatch();

};

#endif // MANAGER_DATA_H_

