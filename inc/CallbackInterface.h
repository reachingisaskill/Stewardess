
#ifndef CALLBACK_INTERFACE_H_
#define CALLBACK_INTERFACE_H_

#include "Definitions.h"
#include "Handle.h"


class Payload;
class Serializer;
class Manager;

class CallbackInterface
{
  // Manager base is a friend so that it can store a pointer to the data here
  friend class Manager;

  private:
    Manager* _manager;

  protected:
    // Return a reference to the manager
    Manager& manager() { return *_manager; }


  public:
    CallbackInterface();
    virtual ~CallbackInterface();


    // Return a new'd serializer object to implement the transfer protocol
    virtual Serializer* buildSerializer() const = 0;


    // Called when the server first starts for initialisation functions
    virtual void onStart() {}


    // Called immediately after the server stops for cleanup
    virtual void onStop() {}


    // Called when a read event is triggered.
    virtual void onRead( Handle, Payload* ) {}


    // Called when a write event is triggered
    virtual void onWrite( Handle ) {}


    // Called when a new connection is added
    virtual void onConnect( Handle ) {}


    // Called when a connection event occurs
    virtual void onConnectionEvent( Handle, ConnectionEvent ) {}


    // Called when a server event occurs
    virtual void onEvent( ServerEvent ) {}


    // Called on every server 'tick'
    virtual void onTick( int ) {}

};

#endif // CALLBACK_INTERFACE_H_

