
#ifndef STEWARDESS_CONNECTION_H_
#define STEWARDESS_CONNECTION_H_

#include "Definitions.h"
#include "LibeventIncludes.h"
#include "Manager.h"
#include "Handle.h"

#include <string>


namespace Stewardess
{

  class Serializer;
  class CallbackInterface;

  class Connection
  {
    private:
      // Count the number of references to this data
      std::atomic<size_t> _references;


      // Store the assigned id
      UniqueID _identifier;

      // Mutex to control key status: closing, events, etc
      std::mutex _theMutex;

      // Flag to trigger the destruction of the connection data
      std::atomic_bool _close;

      // The socket we are connected through
      evutil_socket_t _socket;

      // Pointer to the read and write events
      event* _readEvent;
      event* _writeEvent;
      event* _destroyEvent;


      // Time of creation
      TimeStamp _connectionTime;

      // Last time someone sent/received through this connection
      TimeStamp _lastAccess;
      mutable std::mutex _lastAccessMutex;

    public:

      // Create a new connection and aquire a new id.
      Connection( sockaddr, Manager&, event_base*, evutil_socket_t );
      
      // Destroy buffer event
      ~Connection();

      // Connections are not copyable/moveable after construction
      Connection( const Connection& ) = delete;
      Connection( Connection&& ) = delete;
      Connection operator=( const Connection& ) = delete;
      Connection operator=( Connection&& ) = delete;


      // Addres of the client bound to the socket
      const sockaddr socketAddress;

      // Manager reference
      Manager& manager;

      // Message builder
      Serializer* const serializer;

      // Read buffer stored here so we don't need to keep re-allocating it
      size_t bufferSize;


      // Called by the manager to indicate that the connection handler is ready
      void open( const timeval* = nullptr );


      // Set the close flag to true
      void close();


      // Return true if its not closed
      bool isOpen();


      // Mutex controlled write
      void write( Payload* );

    
      // Return the ID number of its creation
      ConnectionID getConnectionID() const { return (ConnectionID)this; }


      // Return the unique user id for this connection
      UniqueID getIdentifier() const { return _identifier; }

      // Set the unique user id
      void setIdentifier( UniqueID );

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


      // Increment the reference counter
      void incrementReferences();

      // Decrement the reference counter
      void decrementReferences();

  };

}

#endif // STEWARDESS_CONNECTION_H_

