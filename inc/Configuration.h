
#ifndef STEWARDESS_CONFIGURATION_H_
#define STEWARDESS_CONFIGURATION_H_

#include "Definitions.h"

namespace Stewardess
{

  struct ConfigurationData
  {
    // The port number we want
    int portNumber;

    // How often the worker event base forces a tick
    timeval workerTickTime;

    // The minimum time between ticks
    int minTickTime;

    // Modify the internal tick time by this factor
    float tickTimeModifier;

    // Time out time for read/write attempts
    timeval readTimeout;
    timeval writeTimeout;

    // Time out time for forcing the server to shutdown
    timeval deathTime;

    // Set whether the manager can signal connection closes on shutdown
    bool connectionCloseOnShutdown;


    // Default size for the read/write buffers
    size_t bufferSize;

    // Number of parallel threads to handle connection events
    unsigned numThreads;

    // If true a listener event is added to libevent stack to support incoming connections
    bool requestListener;

    // If true a signal event is added to libevent stack to catch the sigint
    bool requestSignalHandler;
  };


  /* 
   * This class is used to configure the server/client process before it starts.
   */
  class Configuration
  {
    friend class Manager;
    private:
      ConfigurationData _data;

    public:
      Configuration( int );
      ~Configuration();


      // Set the number of connection handling threads
      void setNumberThreads( unsigned );


      // Set the default buffer size. Should probably be bigger than the expected payload size
      void setDefaultBufferSize( size_t );


      // Set the timeouts for each connection
      void setReadTimeout( unsigned int );
      void setWriteTimeout( unsigned int );


      // Set the modifier factor for the internal tick time
      void setTickTimeModifier( float );

      // Set the modifier factor for the internal tick time
      void setMinTickTime( unsigned int );

      // Set the time before the connections are forcibly closed once shutdown has been called.
      void setDeathTime( unsigned int );


      // Set where the manager can signal connection close when shutdown is called.
      // Note the connection will only close once another event has triggered. e.g. read timeout.
      void setCloseConnectionsOnShutdown( bool );


      // Set whether a listener event is required to allow new remote connection requests
      void setRequestListener( bool );


      // Set whether a listener event is required to allow new remote connection requests
      void setRequestSignalHandler( bool );

  };

}

#endif // STEWARDESS_CONFIGURATION_H_

