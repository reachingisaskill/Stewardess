
#include "TestClient.h"


TestClient::TestClient() :
  _counter( 0 )
{
}


void TestClient::onStart()
{
  Handle result = manager().connectTo( "0.0.0.0", "7007" );
  if ( ! result )
  {
    manager().abort();
  }
}


void TestClient::onRead( Handle c, Payload* p )
{
  std::cout << "RECEIVED: From connection: " << c.getIDNumber() << '\n' << ((TestPayload*)p)->getMessage() << std::endl;
  delete p;
}


void TestClient::onConnectionEvent( Handle connection, ConnectionEvent event )
{
  switch( event )
  {
    case ConnectionEvent::Connect :
    {
      std::cout << "Connection Event" << std::endl;

      TestPayload p( "Hello" );
      connection.write( &p );
    }
    break;

    default:
    break;
  }
}


void TestClient::onTick( Milliseconds time )
{
  std::cout << "TICK : " << time.count() << std::endl;
}


