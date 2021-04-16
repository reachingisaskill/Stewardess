
#ifndef STEWARDESS_HANDLE_H_
#define STEWARDESS_HANDLE_H_

#include "Definitions.h"


namespace Stewardess
{
    
  class Connection;
  class Payload;
  class InetAddress;


  /*
   * Stores an internal pointer to the connection data.
   *
   * Class is not publically constructable. But must be deleted once the connection is closed
   *  Server requires all handles to be removed before the connection data is cleaned up
   *
   * Handle class acts a reference to the connection data.
   * Reference counting ensures that the connection is only closed once all other 
   *  references are destroyed.
   */
  class Handle
  {
    friend class Connection;
    private:
      // Hidden connection data
      Connection* _connection;


      // Users can't create active handles
      // Create a handle for the requested connection
      Handle( Connection* );

    public:
      // Create a null handle for connections that don't exist
      Handle();
      // Default destructor
      ~Handle();

      // Copy and assignment functons
      Handle( const Handle& );
      Handle( Handle&& );
      Handle& operator=( const Handle& );
      Handle& operator=( Handle&& );


      // Release the handle so that the connection can be fully closed
      void release();


      // Return the address
      const InetAddress& getIPAddress() const;


      // Returns true if the connection is still alive
      bool isOpen() const;


      // Force the connection to close at the next server tick
      void close() const;


      // Writes a payload to the output buffer. Will fail if it is closed
      void write( Payload* ) const;


      // Returns the creation number
      ConnectionID getConnectionID() const;


      // Returns the user id number
      UniqueID getIdentifier() const;


      // Sets the user id number
      void setIdentifier( UniqueID );


      // Return the timestamp when the connection was opened
      TimeStamp creationTime() const;


      // Return the last access
      TimeStamp lastAccess() const;


      // Return false for a dead connection
      bool exists() const { return _connection != nullptr; }


      // Returns false for a dead connection
      operator bool() const { return _connection != nullptr; }
  };

}

#endif // STEWARDESS_HANDLE_H_

