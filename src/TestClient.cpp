
#include "TestClient.h"


namespace Stewardess
{

  TestClient::TestClient() :
    _counter( 0 ),
    _handle(),
    _tickCounter( 0 ),
    _alive( false )
  {
  }


  void TestClient::onStart()
  {
//    _handle = manager().connectTo( "0.0.0.0", "7007" );
//    if ( ! _handle )
//    {
//      manager().abort();
//    }
//    else
//    {
//    }
    manager().requestConnectTo( "0.0.0.0", "7127", 0 );
    _alive = true;

    manager().createTimer( 12, true );
    manager().startTimerCountdown( 12, Milliseconds(3700) );
  }


  void TestClient::onStop()
  {
    _handle.release();
  }


  void TestClient::onRead( Handle c, Payload* p )
  {
    std::cout << "RECEIVED: From connection: " << c.getConnectionID() <<  "  --  " << ((TestPayload*)p)->getMessage() << std::endl;
    delete p;
  }


  void TestClient::onConnectionEvent( Handle connection, ConnectionEvent event, const char* error )
  {
    switch( event )
    {
      case ConnectionEvent::Connect :
      {
        _handle = connection;
        std::cout << "Connection Event" << std::endl;
        std::cout << "Successfully connected to : " << _handle.getIPAddress().getStringFull() << std::endl;

        TestPayload p( "Hello" );
        connection.write( &p );
      }
      break;

      case ConnectionEvent::Disconnect :
      {
        std::cout << "Disconnection Event" << std::endl;

        _alive = false;
        manager().shutdown();
      }
      break;

      case ConnectionEvent::DisconnectError :
      {
        std::cout << "Unexpected Disconnection Event: " << error << std::endl;

        _alive = false;
        manager().shutdown();
      }
      break;

      default:
      break;
    }
  }


  void TestClient::onTick( Milliseconds time )
  {
    if ( _alive && _handle )
    {
      std::cout << std::endl;
      _tickCounter += time.count();
      if ( _tickCounter > 5000 )
      {
        INFO_LOG( "TestClient::OnTick", "Sending message" );
        std::cout << "SENDING: To connection: " << _handle.getConnectionID() <<  "  --  Me again" << std::endl;
        _tickCounter -= 5000;
        TestPayload p( "Me again" );
        _handle.write( &p );
      }
    }
  }


  void TestClient::onTimer( UniqueID timerID )
  {
    std::cout << "OnTime called: " << timerID << std::endl;
  }
}

