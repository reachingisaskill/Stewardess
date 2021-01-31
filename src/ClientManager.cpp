
#include "ClientManager.h"
#include "ManagerData.h"
#include "Connection.h"
#include "ConnectionData.h"
#include "Handler.h"
#include "EventCallbacks.h"
#include "CallbackInterface.h"

#include <csignal>



////////////////////////////////////////////////////////////////////////////////
// ClientManager Member Functions

ClientManager::ClientManager( std::string address, int port_number ) :
  ManagerBase()
{
  _data->portNumber = port_number;
  _data->clientAddress = address;
}


ClientManager::~ClientManager()
{
}


void ClientManager::_run( CallbackInterface& server )
{
  // Set the server pointer
  _data->server = &server;

  // Find the server address
  evutil_addrinfo address_hints;
  evutil_addrinfo* address_answer = nullptr;

  memset( &address_hints, 0, sizeof( address_hints ) );
  address_hints.ai_family = AF_UNSPEC;
  address_hints.ai_socktype = SOCK_STREAM;
  address_hints.ai_protocol = IPPROTO_TCP;
  address_hints.ai_flags = EVUTIL_AI_ADDRCONFIG;

  // Need the port number as a string
  char port_buffer[6];
  evutil_snprintf( port_buffer, sizeof( port_buffer ), "%d", _data->portNumber );

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

  // Configure the event base.
  std::cout << "Creating event_base" << std::endl;
  _data->eventBase = event_base_new();
  if ( _data->eventBase == nullptr )
  {
    EVUTIL_CLOSESOCKET( new_socket );
    free( address_answer );
    throw std::runtime_error( "Could not create a base event. Unknow error." );
  }

  // Create the handler for this job
  Handler handler;

  handler.eventBase = _data->eventBase;
  handler.timeout = defaultTickTime;
  handler.timeoutModifier = _data->tickTimeModifier;
  handler.closeConnections = false;


  // Create an event to force shutdown, but don't enable it
  _data->deathEvent = evtimer_new( _data->eventBase, clientKillTimerCB, (void*)_data );
  if ( _data->deathEvent == nullptr )
  {
    EVUTIL_CLOSESOCKET( new_socket );
    event_base_free( _data->eventBase );
    free( address_answer );
    throw std::runtime_error( "Could not create the death event" );
  }

  // Create a signal event so we can close the system down
  std::cout << "Creating signal event" << std::endl;
  _data->signalEvent = evsignal_new( _data->eventBase, SIGINT, clientInterruptSignalCB, (void*)_data );
  event_add( _data->signalEvent, nullptr );


  // Create a timeout event so we can handle jobs in the background
  std::cout << "Creating timeout event" << std::endl;
  handler.timeoutEvent = evtimer_new( handler.eventBase, workerTimerCB, (void*)&handler );
  evtimer_add( handler.timeoutEvent, &handler.timeout );


  // Create a buffer event, bound to the tcp socket. When freed it will close the socket.
  bufferevent* buffer_event = bufferevent_socket_new( handler.eventBase, new_socket, BEV_OPT_CLOSE_ON_FREE );


  // Create the connection data
  ConnectionData* connectionData = new ConnectionData();
  connectionData->bufferEvent = buffer_event;
  connectionData->server = _data->server;
  connectionData->serializer = _data->server->buildSerializer();
  connectionData->readBuffer.reserve( _data->bufferSize );
  connectionData->writeBuffer.reserve( _data->bufferSize );
  connectionData->close = false;

  // Byte-wise copy the address struct
  std::memcpy( (void*)&connectionData->socketAddress, (void*)address_answer->ai_addr, address_answer->ai_addrlen );

    
  // Make a connection object
  Connection* connection = new Connection( connectionData );
  // Add the new connection to the handler
  handler.connections[ connection->getIDNumber() ] = connection;
  // Signal that something has connected
  connectionData->server->onConnectionEvent( connection, ConnectionEvent::Connect );

  // Set the call back functions
  bufferevent_setcb( buffer_event, bufferReadCB, bufferWriteCB, bufferEventCB, (void*)connection );

  // Set the time outs for reading & writing
  bufferevent_set_timeouts( buffer_event, &_data->readTimeout, &_data->writeTimeout ); 

  // Enable reading & writing on the buffer event
  bufferevent_enable( buffer_event, EV_READ|EV_WRITE );


  // Start the libevent loop using the base event
  std::cout << "Dispatching lister event" << std::endl;
  event_base_dispatch( _data->eventBase );


  // Do some cleanup
  std::cout << "Freeing events" << std::endl;
  bufferevent_free( buffer_event );
  delete connection;
  free( address_answer );
  event_free( handler.timeoutEvent );
  event_free( _data->deathEvent );
  event_free( _data->signalEvent );
  event_base_free( _data->eventBase );
}

