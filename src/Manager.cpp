
#include "Manager.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"
#include "WorkerThread.h"
#include "Connection.h"

#include <cstring>
#include <cmath>


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Member functions definitions

Manager::Manager( const Configuration& config, CallbackInterface& server ) :
  _configuration( config._data ),
  _server( server ),
  _abort( false ),
  _connections(),
  _closedConnections(),
  _eventBase( nullptr ),
  _listener( nullptr ),
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


Seconds Manager::getUpTime() const
{
  return std::chrono::duration_cast<Seconds>( std::chrono::system_clock::now() - _serverStartTime );
}


Handle Manager::requestHandle( UniqueID uid )
{
  GuardLock lk( _connectionsMutex );

  ConnectionMap::iterator found = _connections.find( uid );
  if ( found == _connections.end() )
  {
    return Handle();
  }
  else 
  {
    return found->second->requestHandle();
  }
}


void Manager::run()
{
  // Give the server a reference to this manager.
  _server._manager = this;
  

  // Server starts now!
  _serverStartTime = std::chrono::system_clock::now();


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
    // event_add( _deathEvent, &_deathTime );


    // Create a tick event
    _tickEvent = evtimer_new( _eventBase, tickTimerCB, (void*)this );
    if ( _tickEvent == nullptr )
    {
      throw std::runtime_error( "Could not create the tick event." );
    }
    event_add( _tickEvent, &_tickTime );


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
      info->data.tickTime = _configuration.workerTickTime;

      info->data.eventBase = event_base_new();
      if ( info->data.eventBase == nullptr )
      {
        throw std::runtime_error( "Could not create a worker event base. Unknowm error." );
      }

      info->data.tickEvent = evtimer_new( info->data.eventBase, workerTickTimerCB, &info->data );
      if ( info->data.tickEvent == nullptr )
      {
        throw std::runtime_error( "Could not create the worker tick event." );
      }
      event_add( info->data.tickEvent, &info->data.tickTime );

      info->theThread = std::thread( workerThread, info->data );
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

    // Set the tick time stamp
    _tickTimeStamp = std::chrono::system_clock::now();

    // Start the libevent loop using the base event
    std::cout << "Dispatching primary event" << std::endl;

    _server.onStart();
    if ( ! _abort )
    {
      event_base_dispatch( _eventBase );
    }
    _server.onStop();

    // Delete all the outstanding connections
    for (ConnectionList::iterator it = _closedConnections.begin(); it != _closedConnections.end(); ++it )
    {
      delete (*it);
    }
    _closedConnections.clear();

    for (ConnectionMap::iterator it = _connections.begin(); it != _connections.end(); ++it )
    {
      delete it->second;
    }
    _connections.clear();


    // Join all the worker threads.
    std::cout << "Joining worker threads" << std::endl;
    for ( ThreadVector::iterator it = _threads.begin(); it != _threads.end(); ++it )
    {
      (*it)->theThread.join();
      event_free( (*it)->data.tickEvent );
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


void Manager::shutdown()
{
  // Make the death timer pending
  event_add( _deathEvent, &_configuration.deathTime );

  if ( _listener != nullptr )
  {
    // Disable the listener and signal handler
    evconnlistener_disable( _listener );
  }

  // Disable the signal event. If someone sends it twice we just die.
  evsignal_del( _signalEvent );

  // Trigger the server call back
  _server.onEvent( ServerEvent::Shutdown );
}


void Manager::abort()
{
  // Leave a flag for things to check
  _abort = true;

  // Call on stop function
  _server.onStop();

  // Disable the listener
  if ( _listener != nullptr )
  {
    evconnlistener_disable( _listener );
  }

  // Disable the signal event. If someone sends it twice we just die.
  evsignal_del( _signalEvent );

  // Kill the worker threads
  for ( ThreadVector::iterator it = _threads.begin(); it != _threads.end(); ++it )
  {
    event_base_loopbreak( (*it)->data.eventBase );
  }

  // Kill the manager thread
  event_base_loopbreak( _eventBase );
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
  int result = evutil_getaddrinfo( host.c_str(), port.c_str(), &address_hints, &address_answer );
  if ( result != 0 )
  {
    std::cerr << "Could not resolve hostname" << std::endl;
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
    std::cerr << "Could not create a socket" << std::endl;
    return Handle();
  }

  // Try to connect to the remote host
  std::cout << "Connecting to host." << std::endl;
  if ( connect( new_socket, address_answer->ai_addr, address_answer->ai_addrlen ) )
  {
    EVUTIL_CLOSESOCKET( new_socket );
    while( address_answer != nullptr )
    {
      evutil_addrinfo* temp = address_answer->ai_next;
      free( address_answer );
      address_answer = temp;
    }
    std::cerr << "Failed to connect to server" << std::endl;
    return Handle();
  }

  event_base* worker_base = _threads[ this->getNextThread() ]->data.eventBase;

  // Create a buffer event, bound to the tcp socket. When freed it will close the socket.
  bufferevent* buffer_event = bufferevent_socket_new( worker_base, new_socket, BEV_OPT_CLOSE_ON_FREE );

  // Create the connection 
  Connection* connection = new Connection();
  connection->bufferEvent = buffer_event;
  connection->server = &_server;
  connection->serializer = _server.buildSerializer();
  connection->readBuffer.reserve( _configuration.bufferSize );

  // Byte-wise copy the address struct
  std::memcpy( (void*)&connection->socketAddress, (void*)address_answer->ai_addr, address_answer->ai_addrlen );

  // Clear the address memory
  while( address_answer != nullptr )
  {
    evutil_addrinfo* temp = address_answer->ai_next;
    free( address_answer );
    address_answer = temp;
  }

    
  // Add the new connection to the manager
  this->addConnection( connection );

  // Signal that something has connected
  connection->server->onConnectionEvent( connection->requestHandle(), ConnectionEvent::Connect );


  // Set the call back functions
  bufferevent_setcb( buffer_event, bufferReadCB, bufferWriteCB, bufferEventCB, (void*)connection );

  // Set the time outs for reading & writing only if they're > 0
  bufferevent_set_timeouts( buffer_event, this->getReadTimeout(), this->getWriteTimeout() ); 

  // Enable reading & writing on the buffer event
  bufferevent_enable( buffer_event, EV_READ|EV_WRITE );

  // Return the handle
  return connection->requestHandle();
}


size_t Manager::getNextThread()
{
  GuardLock lk( _nextThreadMutex );

  size_t result = _nextThread++;
  if ( _nextThread == _threads.size() )
  {
    _nextThread = 0;
  }

  return result;
}


const timeval* Manager::getReadTimeout() const
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


const timeval* Manager::getWriteTimeout() const
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
    GuardLock lk( _connectionsMutex );
    num = _connections.size();
  }

  _tickTime.tv_sec = _configuration.minTickTime + _configuration.tickTimeModifier * ( std::log10( num + 1 ) );

  return & _tickTime;
}


void Manager::addConnection( Connection* connection )
{
  GuardLock lk( _connectionsMutex );
  _connections[ connection->getIdentifier() ] = connection;
}


void Manager::closeConnection( Connection* connection )
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
    GuardLock lk( _closedConnectionsMutex );
    _closedConnections.push_back( connection );
  }
}


//void Manager::closeAllConnections()
//{
//  GuardLock lk( _manager._connectionsMutex );
//
//  ConnectionMap::iterator it = _manager._connections.begin();
//  while( it != _manager._connections.end() )
//  {
//    (*it)->close = true;
//  }
//}


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


