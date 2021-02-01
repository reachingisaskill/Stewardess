
#include "Manager.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"
#include "WorkerThread.h"

#include <cstring>


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Member functions definitions

Manager::Manager( const Configuration& config, CallbackInterface& server ) :
  _configuration( config._data ),
  _server( server ),
  _connections(),
  _closedConnections(),
  _eventBase( nullptr ),
  _listener( nullptr ),
  _signalEvent( nullptr ),
  _tickEvent( nullptr ),
  _deathEvent( nullptr ),
  _socketAddress(),
  _tickTime( { 1, 0 } ),
  _threads(),
  _nextThread( 0 )
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

  memset( &_socketAddress, 0, sizeof( _socketAddress ) );
}


Manager::~Manager()
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
  {
    std::cout << "No more managers. Global libevent shutdown." << std::endl;
    libevent_global_shutdown();
  }
}


void Manager::_cleanup()
{
  if ( _deathEvent )
  {
    event_free( _deathEvent );
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


std::string Manager::getIPAddress() const
{
  // 40 Allows for IPv6 + \0
  char address_string[40];
  evutil_inet_ntop( _socketAddress.sin_family, &_socketAddress.sin_addr, address_string, 40 );
  return std::string( address_string );
}


int Manager::getPortNumber() const
{
  return _socketAddress.sin_port;
}


void Manager::run()
{
  // Give the server a reference to this manager.
  _server._manager = this;


  // Configure the socket address
  _socketAddress.sin_family = AF_INET;
  _socketAddress.sin_addr.s_addr = INADDR_ANY;
  _socketAddress.sin_port = htons( _configuration.portNumber );


  try
  {

    // Configure the event base for the control thread
    std::cout << "Creating event_base" << std::endl;
    _eventBase = event_base_new();
    if ( _eventBase == nullptr )
    {
      throw std::runtime_error( "Could not create an event base. Unknown error." );
    }


    // Create an event to force shutdown, but don't enable it
    _deathEvent = evtimer_new( _eventBase, killTimerCB, (void*)this );
    if ( _deathEvent == nullptr )
    {
      throw std::runtime_error( "Could not create the death event." );
    }
    // event_add( data->deathEvent, &data->deathTime );


    // Create a signal event so we can close the system down
    _signalEvent = evsignal_new( _eventBase, SIGINT, interruptSignalCB, (void*)this );
    if ( _signalEvent == nullptr )
    {
      throw std::runtime_error( "Could not create the signal event." );
    }
    event_add( _signalEvent, nullptr );


    // Create the worker threads
    std::cout << "Intialising workers." << std::endl;
    for ( unsigned int i = 0; i < _configuration.numThreads; ++i )
    {
      ThreadInfo* info = new ThreadInfo();

      info->data.eventBase = event_base_new();
      if ( info->data.eventBase == nullptr )
      {
        throw std::runtime_error( "Could not create a worker event base. Unknowm error." );
      }
      info->theThread = std::thread( workerThread, info->data.eventBase );
      _threads.push_back( info );
    }


    // Build a listener if wanted
    if ( _configuration.requestListener )
    {
      std::cout << "Creating listener" << std::endl;
      _listener = evconnlistener_new_bind( _eventBase, listenerAcceptCB, (void*)this,
                                           LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                           (sockaddr*)&_socketAddress, sizeof(_socketAddress) );
      if ( _listener == nullptr )
      {
        throw std::runtime_error( "Could not bind a listener to the requested socket." );
      }
      evconnlistener_set_error_cb( _listener, listenerErrorCB );
    }


    // Start the libevent loop using the base event
    std::cout << "Dispatching primary event" << std::endl;
    event_base_dispatch( _eventBase );


    // Join all the worker threads.
    std::cout << "Joining worker threads" << std::endl;
    for ( ThreadVector::iterator it = _threads.begin(); it != _threads.end(); ++it )
    {
      (*it)->theThread.join();
      event_base_free( (*it)->data.eventBase );
      delete (*it);
    }

  }
  catch( std::exception& ex )
  {
    this->_cleanup();
    _server._manager = nullptr;
    throw ex;
  }

  // Clean up any other events that were created
  this->_cleanup();

  // Remove the reference manager access is only defined while the server is running.
  _server._manager = nullptr;
}




Handle Manager::connectTo( std::string host, std::string port )
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
  int result = evutil_getaddrinfo( _data->clientAddress.c_str(), port_buffer, &address_hints, &address_answer );
  if ( result != 0 )
  {
    std::cerr << "Could not resolve hostname";
    return;
  }

  // Request a socket
  evutil_socket_t new_socket = socket( address_answer->ai_family, address_answer->ai_socktype, address_answer->ai_protocol );
  if ( new_socket < 0 )
  {
    free( address_answer );
    throw std::runtime_error( "Could not create a socket" );
  }

  // Try to connect to the remote host
  std::cout << "Connecting to host." << std::endl;
  if ( connect( new_socket, address_answer->ai_addr, address_answer->ai_addrlen ) )
  {
    EVUTIL_CLOSESOCKET( new_socket );
    free( address_answer );
    std::cerr << "Failed to connect to server";
    return;
  }
}


timeval* Manager::getReadTimeout()
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


timeval* Manager::getWriteTimeout()
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


timeval* Manager::getTickTime()
{
  size_t num;
  {
    GuardLock lk( _manager._connectionsMutex );
    num = _manager._connections.size();
  }

  _tickTime.tv_sec = _manager.configuration.minTickTime + _manager._tickModifier * ( std::log10( num + 1 ) );

  std::cout << "TICK " << _tickTime.tv_sec;

  return & _tickTime;
}


void Manager::addConnection( Connection* connection )
{
  GuardLock lk( _manager->_connectionsMutex );
  _manager->_connections[ connection->getIdentifier() ] = connection;
}


void Manager::closeConnection( Connction* connection )
{
  // Mark the flag
  connection->close();

  // Remove it from the active map
  {
    GuardLock lk( _connectionsMutex );
    ConnectionMap::iterator it = _connections.find( connection->getIdentifier() );
    if ( it != _connections.end() )
    {
      _connections.erase( it );
    }
  }

  // Push it to the closed list
  {
    GuardLock lk( _closedConnections );
    _closedConnections.push_back( connection );
  }
}


void Manager::closeAllConnections()
{
  GuardLock lk( _manager._connectionsMutex );

  ConnectionMap::iterator it = _manager._connections.begin();
  while( it != _manager._connections.end() )
  {
    (*it)->close = true;
  }
}


void Manager::cleanupClosedConnections()
{
  GuardLock lk( _closedConnectionsMutex );
  
  ConnectionList::iterator it = _closedConnections.begin();
  while( it != _closedConnections.end() )
  {
    if ( (*it)->getNumberHandles() == 0 )
    {
      delete (*it);
      it = _closedConnections.erase( it );
    }
    else
    {
      ++it;
    }
  }
}


