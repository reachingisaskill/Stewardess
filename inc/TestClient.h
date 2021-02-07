
#ifndef STEWARDESS_TEST_CLIENT_H_
#define STEWARDESS_TEST_CLIENT_H_

#include "Definitions.h"
#include "CallbackInterface.h"
#include "TestSerializer.h"
#include "Connection.h"


namespace Stewardess
{

  class TestClient : public CallbackInterface
  {
    private:
      unsigned int _counter;
      Handle _handle;
      unsigned long _tickCounter;

      std::atomic<bool> _alive;

    public:
      TestClient();


      // Return a new'd serializer object to implement the transfer protocol
      virtual Serializer* buildSerializer() const override { return new TestSerializer(); }


      // Called when the server first starts for initialisation functions
      virtual void onStart() override;


      // Called when a read event is triggered.
      virtual void onRead( Handle, Payload* ) override;


      // Called when a read event is triggered.
      virtual void onWrite( Handle ) override {}


      // Called when a connection event occurs
      virtual void onConnectionEvent( Handle, ConnectionEvent, const char* ) override;


      // Called every server 'tick' with the elapsed time
      virtual void onTick( Milliseconds ) override;
  };

}

#endif // STEWARDESS_TEST_CLIENT_H_

