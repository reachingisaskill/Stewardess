
#include "Manager.h"
#include "ManagerData.h"
#include "Connection.h"
#include "ConnectionData.h"


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Declare the listener call back and dispatch functions

void dispatchThread( ManagerData* );
void listenerAcceptCB( evconnlistener*, evutil_socket_t, sockaddr*, int, void* );
void listenerErrorCB( evconnlistener*, void* );


////////////////////////////////////////////////////////////////////////////////
// Declare the bufferevent call back and dispatch functions

void bufferReadCB( bufferevent*, void* );
void bufferWriteCB( bufferevent*, void* );
void bufferEventCB( bufferevent*, short, void* );


////////////////////////////////////////////////////////////////////////////////
// Manager Member Functions

Manager::Manager( int port_number, MessageBuilderBase* builder ) :
  _data( nullptr ),
  _messageBuffer()

{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

  _data = new ManagerData( this, port_number );
  _data->messageBuilder = builder;
}


Manager::~Manager()
{
  if ( _data != nullptr )
    delete _data;

  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
    libevent_global_shutdown();
}


void Manager::run()
{
  // Setup and start the libevent listener loop and return
  std::cout << "Calling dispatch" << std::endl;
  _data->dispatch();
}


void Manager::close()
{
}


bool Manager::pop( MessageBase*& message )
{
  return _messageBuffer.waitPop( message );
}


void Manager::push( MessageBase* /*message*/ )
{

}


void Manager::publishMessage( MessageBase* message )
{
  std::cout << "Publishing message" << std::endl;
  // Push the message to the buffer
  _messageBuffer.push( message );
}


////////////////////////////////////////////////////////////////////////////////
// Manager Data Member Functions


ManagerData::ManagerData( Manager* owner, int p ) :
  owner( owner ),
  listenerThread(),
  portNumber( p ),
  socket( 0 ),
  socketAddress(),
  timeout( { 5, 0 } )
{
  // Set the server address
  socketAddress.sin_family = AF_INET;
  socketAddress.sin_port = htons( portNumber );
  socketAddress.sin_addr.s_addr = INADDR_ANY;
}


void ManagerData::dispatch()
{
  // Start a new thread to handle the listener
  listenerThread = std::thread( dispatchThread, this );
}


////////////////////////////////////////////////////////////////////////////////
// Dispatch function

void dispatchThread( ManagerData* management )
{
  std::cout << "  Starting dispatch thread." << std::endl;
  // Configure the event base.
  event_base* base = event_base_new();
  if ( base == nullptr )
  {
    throw std::runtime_error( "Could not create a base event. Unknow error." );
  }

  // Build a listener and bind it to a new socket
  evconnlistener* listener = evconnlistener_new_bind( base, listenerAcceptCB, (void*)management, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (sockaddr*)&management->socketAddress, sizeof(management->socketAddress) );
  if ( listener == nullptr )
  {
    throw std::runtime_error( "Could not bind a listener to the requested socket." );
  }

  // Set the error call back function on the listener
  evconnlistener_set_error_cb( listener, listenerErrorCB );

  std::cout << "  Configured listener. Dispatching." << std::endl;

  // Start the libevent loop using the base event
  event_base_dispatch( base );

  std::cout << "  Freeing listener and event base" << std::endl;

  // Free the listener
  evconnlistener_free( listener );

  // Free the event base
  event_base_free( base );
}


////////////////////////////////////////////////////////////////////////////////
// Listener call back functions

void listenerAcceptCB( evconnlistener* listener, evutil_socket_t new_socket, sockaddr* address, int address_length, void* data )
{
  ManagerData* owner = (ManagerData*)data;
  std::cout << "  Listener Accept Called" << std::endl;

  // Get a copy of the event base
  event_base* base = evconnlistener_get_base( listener );

  // Create a buffer event, bound to the tcp socket. When freed it will close the socket.
  bufferevent* buffer_event = bufferevent_socket_new( base, new_socket, BEV_OPT_CLOSE_ON_FREE );


  // Create the connection data
  ConnectionData* connectionData = new ConnectionData();
  connectionData->bufferEvent = buffer_event;
  connectionData->manager = owner->owner;
  connectionData->messageBuilder = owner->messageBuilder->clone();

  std::memcpy( (void*)&connectionData->socketAddress, (void*)address, address_length );

    
  // Make a connection object so other functions can access the socket
  Connection* connection = new Connection( connectionData );


  // Set the call back functions for the buffer event and pass them a pointer to the connection object
  bufferevent_setcb( buffer_event, bufferReadCB, bufferWriteCB, bufferEventCB, connection );

  // Set the time outs for reading & writing
  bufferevent_set_timeouts( buffer_event, &owner->timeout, &owner->timeout ); 

  // Enable reading & writing on the buffer event
  bufferevent_enable( buffer_event, EV_READ|EV_WRITE );
}


void listenerErrorCB( evconnlistener* /*listener*/, void* /*data*/ )
{
//  ManagerData* owner = (ManagerData*)data;
  std::cout << "  Listener Error Called" << std::endl;

  // Handle listener errors

}


////////////////////////////////////////////////////////////////////////////////
// Buffer event callback functions

void bufferReadCB( bufferevent* event, void* data )
{
  Connection* connection = (Connection*)data;
  ConnectionData* connection_data = connection->getData();
  MessageBuilderBase* builder = connection_data->messageBuilder;
  std::cout << "  Buffer Read called" << std::endl;

  while( true )
  {
    size_t count = bufferevent_read( event, connection_data->rawBuffer, sizeof( connection_data->rawBuffer ) );

    if ( count <= 0 )
    {
      // Error checking?

      // No more data
      break;
    }

    // Iterate through the characters
    for ( size_t pos = 0; pos < count; ++pos )
    {
      // Push to the builder
      builder->build( connection_data->rawBuffer[pos] );
      // If there is a finished message
      if ( builder->isBuilt() )
      {
        connection_data->manager->publishMessage( builder->getMessage() );
      }

      if ( builder->error() )
      {
        // builder->getError();
      }
    }
  }

  if ( builder->isBuilding() )
  {
    // Something clearly broke.
    std::cout << "  ERROR Unfinished message" << std::endl;
  }

  std::cout << "  Finished reading" << std::endl;

  bufferevent_write( event, "Thanks!", sizeof( "Thanks!" ) );

}


void bufferWriteCB( bufferevent* /*event*/, void* /*data*/ )
{
//  Connection* connection = (Connection*)data;
  std::cout << "  Buffer Write called" << std::endl;

  // Nothing to do?
}


void bufferEventCB( bufferevent* event, short flags, void* data )
{
  Connection* connection = (Connection*)data;
  std::cout << "  Buffer Event called" << std::endl;

  if ( flags & BEV_EVENT_ERROR )
  {
    std::cerr << "  ERROR buffer event: " << EVUTIL_SOCKET_ERROR() << std::endl;
    bufferevent_free( event );
    delete connection;
  }

  if ( flags & BEV_EVENT_EOF )
  {
    std::cerr << "  Socket closed?" << std::endl;
    bufferevent_free( event );
    delete connection;
  }
}

