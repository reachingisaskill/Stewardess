
#ifndef STEWARDESS_CALLBACK_INTERFACE_H_
#define STEWARDESS_CALLBACK_INTERFACE_H_

#include "Definitions.h"
#include "Handle.h"
#include "Manager.h"


namespace Stewardess
{

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


      // Called when a connection event occurs
      virtual void onConnectionEvent( Handle, ConnectionEvent, const char* = nullptr ) {}


      // Called when a server event occurs
      virtual void onEvent( ServerEvent, const char* = nullptr ) {}


      // Called on every server 'tick'
      virtual void onTick( Milliseconds ) {}

  };

}

#endif // STEWARDESS_CALLBACK_INTERFACE_H_

