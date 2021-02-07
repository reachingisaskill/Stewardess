
#include "TestServer.h"


namespace Stewardess
{

  void TestServer::onRead( Handle c, Payload* p )
  {
    std::cout << "RECEIVED: From connection: " << c.getIDNumber() <<  "  --  " << ((TestPayload*)p)->getMessage() << std::endl;
    delete p;

    std::cout << "SENDING: To connection: " << c.getIDNumber() <<  "  --  Cheers bruh" << std::endl;
    TestPayload reply( std::string( "Cheers bruh" ) );
    c.write( &reply );
  }


  void TestServer::onConnectionEvent( Handle /*handle*/, ConnectionEvent event, const char* error )
  {
    switch( event )
    {
      case ConnectionEvent::Connect :
      {
        std::cout << "Connection Event" << std::endl;
      }
      break;

      case ConnectionEvent::Disconnect :
      {
        std::cout << "Disconnection Event" << std::endl;
      }
      break;

      case ConnectionEvent::DisconnectError :
      {
        std::cout << "Unexpected Disconnection Event: " << error << std::endl;
      }
      break;

      default:
      break;
    }
  }


  void TestServer::onTick( Milliseconds /*time*/ )
  {
    std::cout << std::endl;
  }

}

