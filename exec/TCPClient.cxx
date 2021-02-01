
#define PORT_NUMBER 7007
#define HOST_ADDRESS "0.0.0.0"

#include "ClientManager.h"
#include "TestClient.h"

#include <iostream>


int main( int, char** )
{
  std::cout << "Building Config" << std::endl;
  Configuration config( PORT_NUMBER );

  // Configure the config
  config.setNumberThreads( 1 );
  config.setDefaultBufferSize( 4096 );
  config.setReadTimeout( 0 );
  config.setWriteTimeout( 3 );
  config.setTickTimeModifier( 1.0 );
  config.setCloseConnectionsOnShutdown( true );


  std::cout << "Building server" << std::endl;
  TestClient the_client;

  config.run( the_client );

  return 0;
}

