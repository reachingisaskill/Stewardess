
#include "TestClient.h"


TestClient::TestClient() :
  _counter( 0 )
{
}


void TestClient::onRead( Payload* p, const Connection* c )
{
  std::cout << "RECEIVED: From connection: " << c->getIDNumber() << '\n' << ((TestPayload*)p)->getMessage() << std::endl;
  delete p;
}


void TestClient::onConnectionEvent( const Connection* connection, ConnectionEvent event )
{
  switch( event )
  {
    case ConnectionEvent::Connect :
    {
      std::cout << "Connection Event" << std::endl;

      TestPayload p( "Hello" );
      connection->write( &p );
    }
    break;

    default:
    break;
  }
}



