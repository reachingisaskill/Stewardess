
#ifndef STEWARDESS_TEST_SERVER_H_
#define STEWARDESS_TEST_SERVER_H_

#include "CallbackInterface.h"
#include "TestSerializer.h"
#include "Connection.h"


namespace Stewardess
{

  class TestServer : public CallbackInterface
  {
    private:

    public:
      // Return a new'd serializer object to implement the transfer protocol
      virtual Serializer* buildSerializer() const override { return new TestSerializer(); }


      // Called when a read event is triggered.
      virtual void onRead( Handle, Payload* ) override;


      // Called when a connection event occurs
      virtual void onConnectionEvent( Handle, ConnectionEvent, const char* ) override;


      // Called every server 'tick' with the elapsed time
      virtual void onTick( Milliseconds ) override;
  };

}

#endif // STEWARDESS_TEST_SERVER_H_

