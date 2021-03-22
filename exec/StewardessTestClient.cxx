
#define PORT_NUMBER 7127
#define HOST_ADDRESS "0.0.0.0"

#include "logtastic.h"

#include "Manager.h"
#include "Configuration.h"
#include "TestClient.h"

#include <iostream>

using namespace Stewardess;


int main( int, char** )
{
  logtastic::init();
  logtastic::setLogFileDirectory( "./log" );
  logtastic::setLogFile( "client_tests.log" );
  logtastic::setMaxFileSize( 100000 );
  logtastic::setMaxNumberFiles( 1 );
  logtastic::setPrintToScreenLimit( logtastic::warn );
  logtastic::setEnableSignalHandling( false );
  logtastic::setFlushOnEveryCall( true );

  logtastic::start( "Stewardess Client Test", STEWARDESS_VERSION_STRING );

  std::cout << "Building Config" << std::endl;
  Configuration config( PORT_NUMBER );

  // Configure the config
  config.setNumberThreads( 1 );
  config.setDefaultBufferSize( 4096 );
  config.setReadTimeout( 0 );
  config.setWriteTimeout( 1 );
  config.setDeathTime( 1 );
  config.setTickTimeModifier( 1.0 );
  config.setCloseConnectionsOnShutdown( true );


  std::cout << "Building client" << std::endl;
  TestClient the_client;

  Manager manager( config, the_client );

  manager.run();

  logtastic::stop();
  return 0;
}

