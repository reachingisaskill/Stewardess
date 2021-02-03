
#include "TestServer.h"


namespace Stewardess
{

  void TestServer::onRead( Handle c, Payload* p )
  {
    std::cout << "RECEIVED: From connection: " << c.getIDNumber() << '\n' << ((TestPayload*)p)->getMessage() << std::endl;
    TestPayload reply( std::string( "Cheers bruh" ) );

    c.write( &reply );

    delete p;
  }


  void TestServer::onConnectionEvent( Handle, ConnectionEvent )
  {
    std::cout << "Connection Event" << std::endl;
  }

}

