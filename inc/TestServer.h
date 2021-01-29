
#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include "ServerState.h"
#include "TestSerializer.h"
#include "Connection.h"


class TestServer : public ServerState
{
  private:

  public:
    // Return a new'd serializer object to implement the transfer protocol
    virtual Serializer* buildSerializer() const override { return new TestSerializer(); }


    // Called when a read event is triggered.
    virtual void onRead( Payload*, const Connection* ) override;

    // Called when a connection event occurs
    virtual void onConnectionEvent( const Connection*, ConnectionEvent ) override;

};

#endif // TEST_SERVER_H_

