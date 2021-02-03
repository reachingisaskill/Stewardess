
#ifndef STEWARDESS_CONNECTION_H_
#define STEWARDESS_CONNECTION_H_

#include "Definitions.h"
#include "LibeventIncludes.h"
#include "Handle.h"
#include "Buffer.h"

#include <string>


namespace Stewardess
{

  class Serializer;
  class CallbackInterface;

  class Connection
  {
    private:
      static UniqueID _idCounter;
      static std::mutex _idCounterMutex;

      // Count the number of references to this data
      ReferenceCounter _references;

      // Store the assigned id
      UniqueID _identifier;


      // Flag to trigger the destruction of the connection data
      bool _close;
      std::mutex _closeMutex;


      // Time of creation
      TimeStamp _connectionTime;

      // Last time someone sent/received through this connection
      TimeStamp _lastAccess;
      mutable std::mutex _lastAccessMutex;

    public:

      // Create a new connection and aquire a new id.
      Connection();
      
      // Destroy buffer event
      ~Connection();

      // Connections are not copyable/moveable after construction
      Connection( const Connection& ) = delete;
      Connection( Connection&& ) = delete;
      Connection operator=( const Connection& ) = delete;
      Connection operator=( Connection&& ) = delete;

      // Pointer to the associated bufferevent
      bufferevent* bufferEvent;

      // Addres of the client bound to the socket
      sockaddr_in socketAddress;

      // Pointer to the server receiving the callbacks
      CallbackInterface* server;

      // Message builder
      Serializer* serializer;

      // Read buffer
      Buffer readBuffer;


      // Set the close flag to true
      void close();


      // Return true if its not closed
      bool isOpen();


      // Return the unique id for this connection
      UniqueID getIdentifier() const { return _identifier; }

      // Returns a new connection object
      Handle requestHandle();

      // Returns the number of handles still alive
      size_t getNumberHandles() const;

      // Return the time the connection was opened
      TimeStamp getCreationTime() const;

      // Signal that an access was made.
      void touchAccess();

      // Return the last time it was accessed
      TimeStamp getAccess() const;

  };

}

#endif // STEWARDESS_CONNECTION_H_

