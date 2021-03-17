
#ifndef STEWARDESS_MANAGER_IMPLEMENTATION_H_
#define STEWARDESS_MANAGER_IMPLEMENTATION_H_

#include "Definitions.h"
#include "LibeventIncludes.h"
#include "Configuration.h"
#include "Handle.h"
#include "ConnectionRequest.h"

#include <queue>


namespace Stewardess
{

  class CallbackInterface;

  class ManagerImpl
  {
    // Make the public interface a friend
    friend class Manager;

    // Connection needs to know some things as the callback argument
    friend class Connection;

    // Callback functions are friends
    friend void listenerAcceptCB( evconnlistener*, evutil_socket_t, sockaddr*, int, void* );
    friend void listenerErrorCB( evconnlistener*, void* );
    friend void interruptSignalCB( evutil_socket_t, short, void* );
    friend void killTimerCB( evutil_socket_t, short, void* );
    friend void tickTimerCB( evutil_socket_t, short, void* );
    friend void connectCB( evutil_socket_t, short, void* );
    friend void readCB( evutil_socket_t, short, void* );
    friend void writeCB( evutil_socket_t, short, void* );
    friend void destroyCB( evutil_socket_t, short, void* );


    private:

      // Pointer to the configuration
      const ConfigurationData& _configuration;

      // The server pointer
      CallbackInterface& _server;

      // Flag to crash out before server starts
      std::atomic<bool> _abort;


      // Map of all the connections currently with the process
      ConnectionMap _connections;
      mutable std::mutex _connectionsMutex;


      // Vector of pending asynchronous connections
      std::queue< ConnectionRequest > _connectionRequests;
      mutable std::mutex _connectionRequestsMutex;


      // Control event base runs listener, signal handling and server ticks runs listener, signal handling and server ticks
      event_base* _eventBase;

      // Pointer to a listener event
      evconnlistener* _listener;

      // Pointer to the connector event
      event* _connectorEvent;

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
      ManagerImpl( const ConfigurationData&, CallbackInterface& );
      virtual ~ManagerImpl();

      ManagerImpl( const ManagerImpl& ) = delete;
      ManagerImpl( ManagerImpl&& ) = delete;
      ManagerImpl& operator=( const ManagerImpl& ) = delete;
      ManagerImpl& operator=( ManagerImpl&& ) = delete;


      // Return the port that we are listening on
      int getPortNumber() const;


      // Return our IP address
      std::string getIPAddress() const;



      // Makes a new connection to the requested host and port number. Blocks while running.
      //  Optional identifier to give the connection object
      Handle connectTo( std::string, std::string, UniqueID = 0 );

      // Requests a new connection to provided host and port number. Runs asynchronously.
      void requestConnectTo( std::string, std::string, UniqueID = 0 );

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

#endif // STEWARDESS_MANAGER_IMPLEMENTATION_H_

