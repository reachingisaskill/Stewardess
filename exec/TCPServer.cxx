
#define PORT_NUMBER 7007

#include "ServerManager.h"
#include "TestServer.h"

#include <exception>
#include <iostream>
#include <thread>
#include <iomanip>


int main( int, char** )
{
  std::cout << "Building manager" << std::endl;
  ServerManager the_manager( PORT_NUMBER );

  std::cout << "Building server" << std::endl;
  TestServer the_server;

  the_manager.run( the_server );

  return 0;
}

