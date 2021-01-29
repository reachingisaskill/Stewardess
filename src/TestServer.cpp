
#include "TestServer.h"


void TestServer::onRead( Payload* p, const Connection* c )
{
  std::cout << "RECEIVED: From connection: " << c->getIDNumber() << '\n' << ((TestPayload*)p)->getMessage() << std::endl;
  TestPayload reply( std::string( "Cheers bruh" ) );

  c->write( &reply );

  delete p;
}


void TestServer::onConnectionEvent( const Connection*, ConnectionEvent )
{
  std::cout << "Connection Event" << std::endl;
}


