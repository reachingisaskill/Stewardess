
#ifndef TEST_CLIENT_H_
#define TEST_CLIENT_H_

#include "Definitions.h"
#include "CallbackInterface.h"
#include "TestSerializer.h"
#include "Connection.h"


class TestClient : public CallbackInterface
{
  private:
    unsigned int _counter;

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
    virtual void onConnectionEvent( Handle, ConnectionEvent ) override;


    virtual void onTick( int ) override {}
};


#endif // TEST_CLIENT_H_

