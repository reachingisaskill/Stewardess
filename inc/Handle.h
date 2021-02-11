
#ifndef STEWARDESS_HANDLE_H_
#define STEWARDESS_HANDLE_H_

#include "Definitions.h"
#include "ReferenceCounter.h"


namespace Stewardess
{
    
  class Connection;
  class Payload;


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
      Connection* _data;

      // Keep track of how many copies there are
      ReferenceCounter _counter;


      // Users can't create active handles
      // Create a handle for the requested connection
      Handle( ReferenceCounter, Connection* );


    public:
      // Create a null handle for connections that don't exist
      Handle();

      // Default destructor
      ~Handle();


      // Return the address
      std::string getIPAddress() const;


      // Return the port number
      int getPortNumber() const;


      // Returns true if the connection is still alive
      bool isOpen() const;


      // Force the connection to close at the next server tick
      void close() const;


      // Writes a payload to the output buffer. Will fail if it is closed
      void write( Payload* ) const;


      // Returns the creation number
      UniqueID getIDNumber() const;


      // Returns the user id number
      UniqueID getIdentifier() const;


      // Sets the user id number
      void setIdentifier( UniqueID );


      // Return the timestamp when the connection was opened
      TimeStamp creationTime() const;


      // Return the last access
      TimeStamp lastAccess() const;


      // Return false for a dead connection
      bool exists() const { return _data != nullptr; }


      // Returns false for a dead connection
      operator bool() const { return _data != nullptr; }
  };

}

#endif // STEWARDESS_HANDLE_H_

