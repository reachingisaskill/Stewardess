
#define PORT_NUMBER 7007

#include "Configuration.h"
#include "TestServer.h"

#include <exception>
#include <iostream>
#include <thread>
#include <iomanip>


int main( int, char** )
{
  std::cout << "Building Config" << std::endl;
  Configuration config( PORT_NUMBER );

  // Configure the config
  config.setNumberThreads( 2 );
  config.setDefaultBufferSize( 4096 );
  config.setReadTimeout( 0 );
  config.setWriteTimeout( 3 );
  config.setTickTimeModifier( 1.0 );
  config.setCloseConnectionsOnShutdown( true );
  config.setRequestListener( true );


  std::cout << "Building server" << std::endl;
  TestServer the_server;

  config.run( the_server );

  return 0;
}

