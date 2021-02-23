
#include "TestHTTPServer.h"


namespace Stewardess
{

  const std::string HTMLHello = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<title>Hello World</title>\n</head>\n<body>Hello World</body>\n</html>";


  void TestHTTPServer::onRead( Handle handle, Payload* p )
  {
    HTTPPayload* payload = (HTTPPayload*)p;
    std::cout << "RECEIVED: From connection: " << handle.getConnectionID() << '\n' << payload->print() << std::endl;
    delete p;

    HTTPPayload response( HTTPPayload::Ok, HTMLHello );
    std::cout << "RESPONSE: " << '\n' << response.print() << std::endl;
    handle.write( &response );
  }


  void TestHTTPServer::onWrite( Handle handle )
  {
    std::cout << "Write complete. Closing connection: " << handle.getConnectionID() << std::endl;
    handle.close();
  }


  void TestHTTPServer::onConnectionEvent( Handle /*handle*/, ConnectionEvent event, const char* error )
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


  void TestHTTPServer::onTick( Milliseconds /*time*/ )
  {
    std::cout << std::endl;
  }

}

