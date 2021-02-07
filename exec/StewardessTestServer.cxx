
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
  logtastic::init();
  logtastic::setLogFileDirectory( "./log" );
  logtastic::setLogFile( "server_tests.log" );
  logtastic::setMaxFileSize( 100000 );
  logtastic::setMaxNumberFiles( 1 );
  logtastic::setPrintToScreenLimit( logtastic::warn );
  logtastic::setEnableSignalHandling( false );
  logtastic::setFlushOnEveryCall( true );

  logtastic::start( "Stewardess Server Test", STEWARDESS_VERSION_STRING );

  std::cout << "Building Config" << std::endl;
  Configuration config( PORT_NUMBER );

  // Configure the config
  config.setNumberThreads( 2 );
  config.setDefaultBufferSize( 4096 );
  config.setReadTimeout( 0 );
  config.setWriteTimeout( 1 );
  config.setDeathTime( 1 );
  config.setTickTimeModifier( 1.0 );
  config.setCloseConnectionsOnShutdown( true );
  config.setRequestListener( true );


  std::cout << "Building server" << std::endl;
  TestServer the_server;

  Manager manager( config, the_server );

  manager.run();

  logtastic::stop();
  return 0;
}

