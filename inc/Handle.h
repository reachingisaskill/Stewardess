
#ifndef HANDLE_H_
#define HANDLE_H_

#include "Definitions.h"
#include "ReferenceCounter.h"


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


    // Return the ID number
    size_t getIDNumber() const;


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


    // Return the timestamp when the connection was opened
    TimeStamp creationTime() const;


    // Return the last access
    TimeStamp lastAccess() const;


    // Returns false for a dead connection
    operator bool() { return _data != nullptr; }
};

#endif // HANDLE_H_

