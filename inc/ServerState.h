
#ifndef SERVER_STATE_H_
#define SERVER_STATE_H_

#include "Definitions.h"


class Payload;
class Serializer;
class Connection;

class ServerState
{
  public:
    virtual ~ServerState() {}


    // Return a new'd serializer object to implement the transfer protocol
    virtual Serializer* buildSerializer() const = 0;


    // Called when a read event is triggered.
    virtual void onRead( Payload*, const Connection* ) {}


    // Called when a write event is triggered
    virtual void onWrite( Payload*, const Connection* ) {}


    // Called when a new connection is added
    virtual void onConnect( const Connection* ) {}


    // Called when a connection event occurs
    virtual void onConnectionEvent( const Connection*, ConnectionEvent ) {}


    // Called when a server event occurs
    virtual void onEvent( ServerEvent, std::string ) {}

};

#endif // SERVER_STATE_H_

