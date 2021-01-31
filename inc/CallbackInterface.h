
#ifndef CALLBACK_INTERFACE_H_
#define CALLBACK_INTERFACE_H_

#include "Definitions.h"


class Payload;
class Serializer;
class Connection;
class ManagerData;

class CallbackInterface
{
  // Manager base is a friend so that it can store a pointer to the data here
  friend class ManagerBase;

  private:
    ManagerData* _managerData;

  protected:
    // Attempts to searche for a connection, returns nullptr if the connection has closed
    Connection* requestConnection( size_t );

    // Returns the number of active connections
    size_t getNumberConnections() const;


  public:
    CallbackInterface();
    virtual ~CallbackInterface();


    // Return a new'd serializer object to implement the transfer protocol
    virtual Serializer* buildSerializer() const = 0;


    // Called when a read event is triggered.
    virtual void onRead( Payload*, const Connection* ) {}


    // Called when a write event is triggered
    virtual void onWrite( const Connection* ) {}


    // Called when a new connection is added
    virtual void onConnect( const Connection* ) {}


    // Called when a connection event occurs
    virtual void onConnectionEvent( const Connection*, ConnectionEvent ) {}


    // Called when a server event occurs
    virtual void onEvent( ServerEvent ) {}


    // Called on every server 'tick'
    virtual void onTick( int ) {}

};

#endif // CALLBACK_INTERFACE_H_

