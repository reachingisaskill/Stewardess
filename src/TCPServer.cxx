
#define PORT_NUMBER 2345

#include "Manager.h"
#include "Connection.h"

#include <exception>
#include <iostream>


int main( int, char** )
{
  std::cout << "Building manager" << std::endl;
  Manager the_manager( PORT_NUMBER );

  std::cout << "Running manager" << std::endl;
  the_manager.run();

  while( true )
  {

    Message message;
    if ( the_manager.pop( message ) )
    {
      std::cout << "Message received: " << message._theMessage << std::endl;

      // Process the message...



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

