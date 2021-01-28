
#define PORT_NUMBER 7007

#include "Manager.h"
#include "Connection.h"
#include "TestMessage.h"

#include <exception>
#include <iostream>
#include <iomanip>


int main( int, char** )
{
  std::cout << "Building manager" << std::endl;
  Manager the_manager( PORT_NUMBER, new TestMessageBuilder() );

  std::cout << "Running manager" << std::endl;
  the_manager.run();


  Event event;
  while( the_manager.pop( event ) )
  {
    switch( event.getEventType() )
    {
      case EventType::Null :
        std::cerr << "Something catastrophic has happened. Fuck..." << std::endl;
        break;

      case EventType::Listener :
        break;

      case EventType::Read :
        {
          TestMessage* message = (TestMessage*)event.readEvent.message;
          std::cout << "Message received: " << message->getMessage() << std::endl;

          std::time_t ts = event.getTimeStamp();
          std::cout << "Message Time Stamp = " << std::put_time( std::localtime( &ts ), "%F %T" ) << std::endl;


          if ( message->getMessage() == "{shutdown}" )
          {
            the_manager.stop();
          }
        }
        break;

      case EventType::Write :
        break;

      case EventType::Connection :
        break;

      case EventType::Server :
        break;
    }

    event.free();
  }

  the_manager.close();

  return 0;
}

