
#include "ManagerData.h"
#include "Manager.h"
#include "ConnectionData.h"
#include "Connection.h"


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
// ManagerData struct memeber functions

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
  connectionData->currentMessage.reserve( RAW_BUFFER_SIZE );
  connectionData->reading = false;

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
      char c = connection_data->rawBuffer[pos];

      if ( connection_data->reading )
      {
        connection_data->currentMessage.push_back( c );

        // If at the end character
        if ( c == '}' )
        {
          // Push the message through the connection for the user to see
          connection_data->manager->publishMessage( Message( connection, connection_data->currentMessage ) );
          // Clear the current message being built
          connection_data->currentMessage.clear();

          // Finished reading message
          connection_data->reading = false;
        }
      }
      else
      {
        // if the start character
        if ( c == '{' )
        {
          connection_data->currentMessage.push_back( c );
          connection_data->reading = true;
        }
      }
    }
  }


  if ( connection_data->reading )
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

