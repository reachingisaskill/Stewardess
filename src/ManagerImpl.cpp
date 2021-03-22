
#include "ManagerImpl.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"
#include "WorkerThread.h"
#include "Connection.h"
#include "TimerData.h"
#include "Exception.h"

#include <signal.h>
#include <cstring>
#include <cmath>


namespace Stewardess
{

  ////////////////////////////////////////////////////////////////////////////////
  // Member functions definitions

  ManagerImpl::ManagerImpl( const ConfigurationData& config, CallbackInterface& server ) :
    _configuration( config ),
    _server( server ),
    _abort( false ),
    _connections(),
    _userTimers(),
    _eventBase( nullptr ),
    _listener( nullptr ),
    _connectorEvent( nullptr ),
    _signalEvent( nullptr ),
    _tickEvent( nullptr ),
    _deathEvent( nullptr ),
    _socketAddress(),
    _tickTime( { 1, 0 } ),
    _tickTimeStamp(),
    _serverStartTime(),
    _threads(),
    _nextThread( 0 )
  {
    memset( &_socketAddress, 0, sizeof( _socketAddress ) );
  }


  ManagerImpl::~ManagerImpl()
  {
  }


  void ManagerImpl::_cleanup()
  {
    {
      GuardLock lk( _connectionsMutex );
      // Delete all the outstanding connections
      for (ConnectionMap::iterator it = _connections.begin(); it != _connections.end(); ++it )
      {
        delete it->second;
      }
      _connections.clear();
    }


    // Join all the worker threads.
    INFO_LOG( "Stewardess::Manager", "Joining worker threads" );
    for ( ThreadVector::iterator it = _threads.begin(); it != _threads.end(); ++it )
    {
      (*it)->theThread.join();
      event_base_free( (*it)->data.eventBase );
      delete (*it);
    }
    _threads.clear();


    // Clear all the user timers
    for ( TimerMap::iterator it = _userTimers.begin(); it != _userTimers.end(); ++it )
    {
      event_free( it->second->theEvent );
      delete it->second;
    }
    _userTimers.clear();


    if ( _deathEvent )
    {
      event_free( _deathEvent );
    }
    if ( _connectorEvent )
    {
      event_free( _connectorEvent );
    }
    if ( _tickEvent )
    {
      event_free( _tickEvent );
    }
    if ( _signalEvent )
    {
      event_free( _signalEvent );
    }
    if ( _listener )
    {
      evconnlistener_free( _listener );
    }
    if ( _eventBase )
    {
      event_base_free( _eventBase );
    }
  }


  std::string ManagerImpl::getIPAddress() const
  {
    // 40 Allows for IPv6 + \0
    char address_string[40];
    evutil_inet_ntop( _socketAddress.sin_family, &_socketAddress.sin_addr, address_string, 40 );
    return std::string( address_string );
  }


  int ManagerImpl::getPortNumber() const
  {
    return ntohs( _socketAddress.sin_port );
  }


  void ManagerImpl::run()
  {
    // Server starts now!
    _serverStartTime = std::chrono::system_clock::now();

    // Configure the socket address
    _socketAddress.sin_family = AF_INET;
    _socketAddress.sin_addr.s_addr = INADDR_ANY;
    _socketAddress.sin_port = htons( _configuration.portNumber );

    try
    {
      // Configure the event base for the control thread
      INFO_LOG( "Stewardess::Manager", "Configuring network logic." );
      _eventBase = event_base_new();
      if ( _eventBase == nullptr )
      {
        throw Exception( "Could not create an event base. Unknown error." );
      }


      // Create an event to force shutdown, but don't enable it
      _deathEvent = evtimer_new( _eventBase, killTimerCB, (void*)this );
      if ( _deathEvent == nullptr )
      {
        throw Exception( "Could not create the death event." );
      }
      // event_add( _deathEvent, &_deathTime );


      // Create a connector timer event
      _connectorEvent = evtimer_new( _eventBase, connectCB, (void*)this );
      if ( _connectorEvent == nullptr )
      {
        throw Exception( "Could not create the connector event." );
      }
      // event_add( _connectorEvent, &immediately );


      // Create a tick event
      _tickEvent = evtimer_new( _eventBase, tickTimerCB, (void*)this );
      if ( _tickEvent == nullptr )
      {
        throw Exception( "Could not create the tick event." );
      }
      event_add( _tickEvent, &_tickTime );


      // Create a signal event so we can close the system down
      if ( _configuration.requestSignalHandler )
      {
        _signalEvent = evsignal_new( _eventBase, SIGINT, interruptSignalCB, (void*)this );
        if ( _signalEvent == nullptr )
        {
          throw Exception( "Could not create the signal event." );
        }
        event_add( _signalEvent, nullptr );
      }


      // Build a listener if wanted
      if ( _configuration.requestListener )
      {
        INFO_STREAM( "Stewardess::Manager" ) << "Configuring listener on port " << _configuration.portNumber;
        _listener = evconnlistener_new_bind( _eventBase, listenerAcceptCB, (void*)this,
                                             LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                             (sockaddr*)&_socketAddress, sizeof(_socketAddress) );
        if ( _listener == nullptr )
        {
          throw Exception( "Could not bind a listener to the requested socket." );
        }
        evconnlistener_set_error_cb( _listener, listenerErrorCB );
      }


      // Create the worker threads
      INFO_LOG( "Stewardess::Manager", "Intialising worker threads." );
      for ( unsigned int i = 0; i < _configuration.numThreads; ++i )
      {
        ThreadInfo* info = new ThreadInfo();
        info->data.tickTime = _configuration.workerTickTime;

        info->data.eventBase = event_base_new();
        if ( info->data.eventBase == nullptr )
        {
          throw Exception( "Could not create a worker event base. Unknown error." );
        }

        info->theThread = std::thread( workerThread, info->data );
        _threads.push_back( info );
      }


      // Set the tick time stamp
      _tickTimeStamp = std::chrono::system_clock::now();

      // Start the libevent loop using the base event
      INFO_LOG( "Stewardess::Manager", "Operation start." );

      _server.onStart();
      if ( ! _abort )
      {
        event_base_dispatch( _eventBase );
      }
      _server.onStop();

      INFO_LOG( "Stewardess::Manager", "Operation stopped." );
    }
    catch( const Exception& ex )
    {
      this->abort();
      this->_cleanup();
      throw ex;
    }

    // Clean up any other events that were created
    this->_cleanup();
  }


  void ManagerImpl::shutdown()
  {
    INFO_LOG( "Stewardess::Manager", "Shutdown requested" );

    // Make the death timer pending
    event_add( _deathEvent, &_configuration.deathTime );

    if ( _listener != nullptr )
    {
      // Disable the listener
      evconnlistener_disable( _listener );
    }

    // Disable the signal event. If someone sends it twice we just die.
    if ( _signalEvent != nullptr )
    {
      evsignal_del( _signalEvent );
    }

    // Trigger the server call back
    _server.onEvent( ServerEvent::Shutdown );
  }


  void ManagerImpl::abort()
  {
    INFO_LOG( "Stewardess::Manager", "Aborting" );

    // Leave a flag for things to check
    _abort = true;

    // Disable the listener
    if ( _listener != nullptr )
    {
      evconnlistener_disable( _listener );
    }

    // Disable the signal event. If someone sends it twice we just die.
    if ( _signalEvent != nullptr )
    {
      evsignal_del( _signalEvent );
    }

    // Kill the worker threads
    for ( ThreadVector::iterator it = _threads.begin(); it != _threads.end(); ++it )
    {
      std::cout << "Breaking worker" << std::endl;
      event_base_loopbreak( (*it)->data.eventBase );
    }

    // Kill the manager thread
    event_base_loopbreak( _eventBase );
  }


  Handle ManagerImpl::connectTo( std::string host, std::string port, UniqueID id )
  {
    // Find the server address
    evutil_addrinfo address_hints;
    evutil_addrinfo* address_answer = nullptr;

    memset( &address_hints, 0, sizeof( address_hints ) );
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    address_hints.ai_protocol = IPPROTO_TCP;
    address_hints.ai_flags = EVUTIL_AI_ADDRCONFIG;


    // Resolve the hostname
    int result = evutil_getaddrinfo( host.c_str(), port.c_str(), &address_hints, &address_answer );
    if ( result != 0 )
    {
      ERROR_STREAM( "Stewardess::Manager" ) << "Could not resolve hostname: " << host;
      return Handle();
    }

    // Request a socket
    evutil_socket_t new_socket = socket( address_answer->ai_family, address_answer->ai_socktype, address_answer->ai_protocol );
    if ( new_socket < 0 )
    {
      while( address_answer != nullptr )
      {
        evutil_addrinfo* temp = address_answer->ai_next;
        free( address_answer );
        address_answer = temp;
      }
      ERROR_LOG( "Stewardess::Manager", "Failed to create a socket" );
      return Handle();
    }

    // Try to connect to the remote host
    INFO_STREAM( "Stewardess::Manager" ) << "Connecting to host: " << host;
    if ( connect( new_socket, address_answer->ai_addr, address_answer->ai_addrlen ) )
    {
      EVUTIL_CLOSESOCKET( new_socket );
      while( address_answer != nullptr )
      {
        evutil_addrinfo* temp = address_answer->ai_next;
        free( address_answer );
        address_answer = temp;
      }
      ERROR_STREAM( "Stewardess::Manager" ) << "Failed to connect to server " << host << ":" << port;
      return Handle();
    }

    // Make the socket non-blocking
    evutil_make_socket_nonblocking( new_socket );

    event_base* worker_base;
    if ( _threads.size() == 0 )
    {
      worker_base = _eventBase;
    }
    else
    {
      worker_base = _threads[ this->getNextThread() ]->data.eventBase;
    }

    // Create the connection 
    Connection* connection = new Connection( *address_answer->ai_addr, *this, worker_base, new_socket );
    connection->setIdentifier( id );
    connection->bufferSize =  _configuration.bufferSize;


    // Clear the address memory
    while( address_answer != nullptr )
    {
      evutil_addrinfo* temp = address_answer->ai_next;
      free( address_answer );
      address_answer = temp;
    }

      
    // Add the new connection to the manager
    this->addConnection( connection );

    // We don't call this function as this is not executed asynchronously - the user knows immediately
//    // Signal that something has connected
//    _server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Connect );

    // Return the handle
    return connection->requestHandle();
  }



  void ManagerImpl::requestConnectTo( std::string address, std::string port, UniqueID uid )
  {
    // Create the request
    ConnectionRequest request = { address, port, uid };

    // Push it to the vector
    GuardLock lk( _connectionRequestsMutex );
    _connectionRequests.push( request );

    // Make sure the event is pending
    event_add( _connectorEvent, &immediately );
  }



  size_t ManagerImpl::getNumberConnections() const
  {
    GuardLock lk( _connectionsMutex );
    return _connections.size();
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Private Member Functions

  size_t ManagerImpl::getNextThread()
  {
    GuardLock lk( _nextThreadMutex );

    size_t result = _nextThread++;
    if ( _nextThread == _threads.size() )
    {
      _nextThread = 0;
    }

    return result;
  }


  const timeval* ManagerImpl::getReadTimeout() const
  {
    if ( _configuration.readTimeout.tv_sec == 0 )
    {
      return nullptr;
    }
    else
    {
      return &_configuration.readTimeout;
    }
  }


  const timeval* ManagerImpl::getWriteTimeout() const
  {
    if ( _configuration.writeTimeout.tv_sec == 0 )
    {
      return nullptr;
    }
    else
    {
      return &_configuration.writeTimeout;
    }
  }


  timeval* ManagerImpl::getTickTime()
  {
    size_t num;
    {
      GuardLock lk( _connectionsMutex );
      num = _connections.size();
    }

    _tickTime.tv_sec = _configuration.minTickTime + _configuration.tickTimeModifier * ( std::log10( num + 1 ) );

    return & _tickTime;
  }


  void ManagerImpl::addConnection( Connection* connection )
  {
    GuardLock lk( _connectionsMutex );
    _connections[ connection->getConnectionID() ] = connection;
    connection->open();
  }


  void ManagerImpl::closeConnection( Connection* connection )
  {
    GuardLock lk( _connectionsMutex );
    ConnectionMap::iterator it = _connections.find( connection->getConnectionID() );
    if ( it != _connections.end() )
    {
      delete it->second;
      _connections.erase( it );
    }
    else
    {
      WARN_STREAM( "ManagerImpl::CloseConnection" ) << "Connection requested closing before initialised: " << connection->getConnectionID();
    }
  }


  void ManagerImpl::createTimer( UniqueID uid, bool repeat )
  {
    TimerData* timer = new TimerData( { this, uid, nullptr, repeat, {0, 0} } );

    timer->theEvent = evtimer_new( _eventBase, userTimerCB, (void*)timer );
    if ( timer->theEvent == nullptr )
    {
      throw Exception( "Could not create user timer event." );
    }

    GuardLock lk( _userTimersMutex );
    TimerMap::iterator found = _userTimers.find( uid );
    if ( found != _userTimers.end() )
    {
      throw Exception( "Cannot create a timer using an exist ID." );
    }

    _userTimers.insert( std::make_pair( uid, timer ) );
  }


  void ManagerImpl::startTimerClock( UniqueID uid, TimeStamp timestamp )
  {
    auto now = std::chrono::system_clock::now();
    if ( timestamp < now )
    {
      WARN_STREAM( "Manager::StartTimer" ) << "Cannot start timer for a negative amount of time. ID: " << uid;
    }

    GuardLock lk( _userTimersMutex );

    TimerMap::iterator found = _userTimers.find( uid );
    if ( found == _userTimers.end() )
    {
      WARN_STREAM( "Manager::StartTimer" ) << "Cannot start timer of unknown ID: " << uid;
      return;
    }

    found->second->time = convertToTimeval( timestamp - now );

    event_add( found->second->theEvent, &found->second->time );
  }


  void ManagerImpl::startTimerCountdown( UniqueID uid, Milliseconds countdown )
  {
    if ( countdown.count() < 0 )
    {
      WARN_STREAM( "Manager::StartTimer" ) << "Cannot start timer for a negative amount of time. ID: " << uid;
    }

    GuardLock lk( _userTimersMutex );

    TimerMap::iterator found = _userTimers.find( uid );
    if ( found == _userTimers.end() )
    {
      WARN_STREAM( "Manager::StartTimer" ) << "Cannot start timer of unknown ID: " << uid;
      return;
    }

    found->second->time = convertToTimeval( countdown );

    event_add( found->second->theEvent, &found->second->time );
  }

}

