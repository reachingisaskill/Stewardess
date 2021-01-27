
#ifndef CONNECTION_DATA_H_
#define CONNECTION_DATA_H_

#include <netinet/in.h>
#include <event2/bufferevent.h>

#include <string>


#ifndef RAW_BUFFER_SIZE
#define RAW_BUFFER_SIZE 4096
#endif


class Manager;

struct ConnectionData
{
  // Pointer to the associated buffer event
  bufferevent* bufferEvent;

  // Addres of the client bound to the socket
  sockaddr_in socketAddress;

  // Pointer to the owning manager
  Manager* manager;

  // The message currently being constructed
  std::string currentMessage;

  // The raw char stream being processed
  char rawBuffer[ RAW_BUFFER_SIZE ];

  // Flag to indicate that we are halfway through a message
  bool reading;
};

#endif // CONNECTION_DATA_H_

