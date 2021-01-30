
#ifndef CONNECTION_DATA_H_
#define CONNECTION_DATA_H_

#include "Definitions.h"
#include "LibeventIncludes.h"
#include "Buffer.h"

#include <string>



class Handler;
class Serializer;
class ServerState;

struct ConnectionData
{
  // Pointer to the connection owner
  Handler* handler;

  // Pointer to the associated bufferevent
  bufferevent* bufferEvent;

  // Addres of the client bound to the socket
  sockaddr_in socketAddress;

  // Pointer to the server receiving the callbacks
  ServerState* server;

  // Message builder
  Serializer* serializer;

  // Input and output buffers
  Buffer readBuffer;
  Buffer writeBuffer;

  // Flag to trigger the destruction of the connection data
  bool close;
};

#endif // CONNECTION_DATA_H_

