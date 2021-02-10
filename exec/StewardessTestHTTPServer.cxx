
#define PORT_NUMBER 8080

#include "Manager.h"
#include "Configuration.h"
#include "TestHTTPServer.h"
#include "Exception.h"

#include <exception>
#include <iostream>
#include <thread>
#include <iomanip>

using namespace Stewardess;


int main( int, char** )
{
  logtastic::init();
  logtastic::setLogFileDirectory( "./log" );
  logtastic::setLogFile( "http_server_tests.log" );
  logtastic::setMaxFileSize( 100000 );
  logtastic::setMaxNumberFiles( 1 );
  logtastic::setPrintToScreenLimit( logtastic::warn );
  logtastic::setEnableSignalHandling( false );
  logtastic::setFlushOnEveryCall( true );

  logtastic::start( "Stewardess HTTP Server Test", STEWARDESS_VERSION_STRING );

  try
  {
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
    config.setRequestListener( true );


    std::cout << "Building server" << std::endl;
    TestHTTPServer the_server;

    Manager manager( config, the_server );

    std::cout << "Running" << std::endl;
    manager.run();
  }
  catch( const Exception& ex )
  {
    std::cerr << "Stewardess error occured: " << ex.what() << std::endl;
  }
  catch( const std::exception& ex )
  {
    std::cerr << "Unexpected error occured: " << ex.what() << std::endl;
  }

  logtastic::stop();
  return 0;
}

