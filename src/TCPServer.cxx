
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

  while( true )
  {

    MessageBase* message_base;
    if ( the_manager.pop( message_base ) )
    {
      TestMessage* message = (TestMessage*)message_base;
      std::cout << "Message received: " << message->getMessage() << std::endl;

      std::time_t ts = message->getTimeStamp();
      std::cout << "Message Time Stamp = " << std::put_time( std::localtime( &ts ), "%F %T" ) << std::endl;

      // Process the message...



      delete message_base;
    }
    else // Some other signal
    {
      // Not implemented yet
      std::cout << "Error occured. Breaking out." << std::endl;
      break;
    }

  }

  the_manager.close();

  return 0;
}

