
#ifndef STEWARDESS_MANAGER_H_
#define STEWARDESS_MANAGER_H_

#include "Definitions.h"
#include "LibeventIncludes.h"
#include "Configuration.h"
#include "Handle.h"


namespace Stewardess
{

  class CallbackInterface;

  class Manager
  {
    // Callback functions are friends
    friend void listenerAcceptCB( evconnlistener*, evutil_socket_t, sockaddr*, int, void* );
    friend void listenerErrorCB( evconnlistener*, void* );
    friend void interruptSignalCB( evutil_socket_t, short, void* );
    friend void killTimerCB( evutil_socket_t, short, void* );
    friend void tickTimerCB( evutil_socket_t, short, void* );


    private:
      // Count the number of managers alive
      static size_t _instanceCount;
      static std::mutex _instanceCountMutex;


      // Pointer to the configuration
      const ConfigurationData& _configuration;

      // The server pointer
      CallbackInterface& _server;

      // Flag to crash out before server starts
      bool _abort;


      // Map of all the connections currently with the process
      ConnectionMap _connections;
      mutable std::mutex _connectionsMutex;

      // Queue of the connection that have been signalled to close
      ConnectionList _closedConnections;
      mutable std::mutex _closedConnectionsMutex;


      // Control event base runs listener, signal handling and server ticks runs listener, signal handling and server ticks
      event_base* _eventBase;

      // Pointer to a listener event
      evconnlistener* _listener;

      // Pointer to the signal event
      event* _signalEvent;

      // Pointer to the timer 'tick' event
      event* _tickEvent;

      // Pointer to the signal event
      event* _deathEvent;


      // Our address
      sockaddr_in _socketAddress;
    
      // The socket we're listening on
      evutil_socket_t _socket;


      // How long the timeout lasts for the next 'tick'
      timeval _tickTime;

      // The last tick time
      TimeStamp _tickTimeStamp;

      // The time the server was started
      TimeStamp _serverStartTime;


      // All the threads
      ThreadVector _threads;

      // The next thread to allocate a connection to
      size_t _nextThread;
      std::mutex _nextThreadMutex;



      // Anything that's not null gets free'd
      void _cleanup();


      // Update and return the next thread index
      size_t getNextThread();

      // Return appropriate pointers for the read and write timeouts
      const timeval* getReadTimeout() const;
      const timeval* getWriteTimeout() const;

      // Add a newly created connection to the map
      void addConnection( Connection* );

      // Deletes connections that are closed and have no handles remaining
      void cleanupClosedConnections();

      // Calculate and return a next tick time
      timeval* getTickTime();

      // Move the connection from the active map to the closed list
      void closeConnection( Connection* );


    public:
      Manager( const Configuration&, CallbackInterface& );
      virtual ~Manager();

      Manager( const Manager& ) = delete;
      Manager( Manager&& ) = delete;
      Manager& operator=( const Manager& ) = delete;
      Manager& operator=( Manager&& ) = delete;


      // Return the port that we are listening on
      int getPortNumber() const;


      // Return our IP address
      std::string getIPAddress() const;


      // Returns true if everything is to run on the primary thread
      bool singleThreadMode() const { return _threads.size() == 0; }


      // Return the timestamp of the last server tick
      TimeStamp getTickTime() const { return _tickTimeStamp; }

      // Return the time the server was started
      TimeStamp getStartTime() const { return _serverStartTime; }

      // Return the uptime in seconds
      Seconds getUpTime() const;


      // Return a pointer to the requested connection
      Handle requestHandle( UniqueID );


      // Makes a new connection to the requested host and port number. Blocks while running.
      Handle connectTo( std::string, std::string );

      // Requests a new connection to provided host and port number. Runs asynchronously.
      void requestConnectTo( std::string, std::string );

      // Returns the number of current active connections
      size_t getNumberConnections() const;


      // Runs the server instance
      void run();


      // Triggers the shutdown proceedure manually
      void shutdown();


      // Forces everything to close immediately. Not recommended.
      void abort();
  };

}

#endif // MANAGER_H_

