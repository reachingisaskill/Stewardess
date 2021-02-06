
#define PORT_NUMBER 7007

#include "Manager.h"
#include "Configuration.h"
#include "TestServer.h"

#include <exception>
#include <iostream>
#include <thread>
#include <iomanip>

using namespace Stewardess;


int main( int, char** )
{
  std::cout << "Building Config" << std::endl;
  Configuration config( PORT_NUMBER );

  // Configure the config
  config.setNumberThreads( 2 );
  config.setDefaultBufferSize( 4096 );
  config.setReadTimeout( 0 );
  config.setWriteTimeout( 1 );
  config.setTickTimeModifier( 1.0 );
  config.setCloseConnectionsOnShutdown( true );
  config.setRequestListener( true );


  std::cout << "Building server" << std::endl;
  TestServer the_server;


  Manager manager( config, the_server );

  manager.run();

  return 0;
}

