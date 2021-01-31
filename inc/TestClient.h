
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


    // Called when a read event is triggered.
    virtual void onRead( Payload*, const Connection* ) override;


    // Called when a read event is triggered.
    virtual void onWrite( const Connection* ) override {}


    // Called when a connection event occurs
    virtual void onConnectionEvent( const Connection*, ConnectionEvent ) override;


    virtual void onTick( int ) override {}
};


#endif // TEST_CLIENT_H_

