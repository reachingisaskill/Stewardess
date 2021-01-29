
#ifndef CONNECTION_DATA_H_
#define CONNECTION_DATA_H_

#include <netinet/in.h>
#include <event2/bufferevent.h>

#include <string>


#ifndef RAW_BUFFER_SIZE
#define RAW_BUFFER_SIZE 4096
#endif


class Manager;
class Serializer;
class ServerState;

struct ConnectionData
{
  // Pointer to the associated buffer event
  bufferevent* bufferEvent;

  // Addres of the client bound to the socket
  sockaddr_in socketAddress;

  // Pointer to the server receiving the callbacks
  ServerState* server;

  // Message builder
  Serializer* serializer;

  // The raw char stream being processed
  char rawBuffer[ RAW_BUFFER_SIZE ];

  // Flag to trigger the destruction of the connection data
  bool close;
};

#endif // CONNECTION_DATA_H_

