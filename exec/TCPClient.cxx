
#define PORT_NUMBER 7007
#define HOST_ADDRESS "0.0.0.0"

#include "ClientManager.h"
#include "TestClient.h"

#include <iostream>


int main( int, char** )
{
  std::cout << "Building manager" << std::endl;
  ClientManager the_manager( HOST_ADDRESS, PORT_NUMBER );

  the_manager.setReadTimeout( 0 );

  std::cout << "Building server" << std::endl;
  TestClient the_client;

  the_manager.run( the_client );

  return 0;
}

