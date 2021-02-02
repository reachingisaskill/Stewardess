
#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include "CallbackInterface.h"
#include "TestSerializer.h"
#include "Connection.h"


class TestServer : public CallbackInterface
{
  private:

  public:
    // Return a new'd serializer object to implement the transfer protocol
    virtual Serializer* buildSerializer() const override { return new TestSerializer(); }


    // Called when a read event is triggered.
    virtual void onRead( Handle, Payload* ) override;


    // Called when a connection event occurs
    virtual void onConnectionEvent( Handle, ConnectionEvent ) override;

};

#endif // TEST_SERVER_H_

