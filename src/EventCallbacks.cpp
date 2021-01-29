
#include "EventCallbacks.h"
#include "ServerState.h"
#include "Manager.h"
#include "ManagerData.h"
#include "Connection.h"
#include "ConnectionData.h"
#include "Handler.h"
#include "Serializer.h"


void connectionUpdate( Connection* );


////////////////////////////////////////////////////////////////////////////////
// Listener call back functions

void listenerAcceptCB( evconnlistener* /*listener*/, evutil_socket_t new_socket, sockaddr* address, int address_length, void* arg )
{
  ManagerData* data = (ManagerData*)arg;
  std::cout << "  Listener Accept Called " << std::endl;

  // Get a copy of the event base
//  event_base* base = evconnlistener_get_base( listener );

  if ( data->threads.size() == 0 )
  {
    std::cerr << "  WHERE ARE MY FUCKING HANDLERS?!" << std::endl;
  }

  // Choose a worker to handle it
  // Testing, first one will do!
  Handler* the_handler = data->threads.front();

  // Lock the handler while we add a buffer event to the event_base
  UniqueLock lock( the_handler->mutex );



  // Create a buffer event, bound to the tcp socket. When freed it will close the socket.
  bufferevent* buffer_event = bufferevent_socket_new( the_handler->eventBase, new_socket, BEV_OPT_CLOSE_ON_FREE );


  // Create the connection data
  ConnectionData* connectionData = new ConnectionData();
  connectionData->bufferEvent = buffer_event;
  connectionData->server = data->server;
  connectionData->serializer = data->server->buildSerializer();
  connectionData->close = false;

  std::memcpy( (void*)&connectionData->socketAddress, (void*)address, address_length );

    
  // Make a connection object
  Connection* connection = new Connection( connectionData );

  // Add the new connection to the handler
  the_handler->connections[ connection->getIDNumber() ] = connection;

  // Set the call back functions for the buffer event and pass them a pointer to the connection object
  bufferevent_setcb( buffer_event, bufferReadCB, bufferWriteCB, bufferEventCB, connection );

  // Set the time outs for reading & writing
  bufferevent_set_timeouts( buffer_event, &data->timeout, &data->timeout ); 

  // Signal that something has connected
  connectionData->server->onConnectionEvent( connection, ConnectionEvent::Connect );

  // Enable reading & writing on the buffer event
  bufferevent_enable( buffer_event, EV_READ|EV_WRITE );


  // Release the handler
  lock.unlock();
}


void listenerErrorCB( evconnlistener* /*listener*/, void* /*data*/ )
{
//  ManagerData* owner = (ManagerData*)data;
  std::cout << "  Listener Error Called" << std::endl;

  // Handle listener errors

}


////////////////////////////////////////////////////////////////////////////////
// Signal event call backs

void interruptSignalCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  ManagerData* data = (ManagerData*)arg;

  std::cerr << "Interrupt received. Time to go." << std::endl;

  // Set the close flag on the handlers
  for ( ThreadList::iterator it = data->threads.begin(); it != data->threads.end(); ++it )
  {
    GuardLock lock( (*it)->mutex );
    event_base_loopexit( (*it)->eventBase, nullptr );
  }

  evconnlistener_disable( data->listener );
  evsignal_del( data->signalEvent );
}


void workerTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  Handler* handler = (Handler*)arg;

  std::cerr << "Worker timer received." << std::endl;

  event_add( handler->timeoutEvent, &handler->timeout);

//  event_del( handler->signalEvent );
}


////////////////////////////////////////////////////////////////////////////////
// Buffer event callback functions

void bufferReadCB( bufferevent* event, void* arg )
{
  Connection* connection = (Connection*)arg;
  ConnectionData* connection_data = connection->getData();
  Serializer* builder = connection_data->serializer;
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
        connection_data->server->onRead( builder->getPayload(), connection );
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

  connectionUpdate( connection );
}


void bufferWriteCB( bufferevent* /*event*/, void* data )
{
  Connection* connection = (Connection*)data;
  std::cout << "  Buffer Write called" << std::endl;

  // Nothing to do?

  connectionUpdate( connection );
}


void bufferEventCB( bufferevent* /*event*/, short flags, void* data )
{
  Connection* connection = (Connection*)data;
  std::cout << "  Buffer Event called" << std::endl;

  if ( flags & BEV_EVENT_CONNECTED )
  {
    std::cerr << "Connection succeeded" << std::endl;
    connection->getData()->server->onConnectionEvent( connection, ConnectionEvent::Connect );
  }

  if ( flags & BEV_EVENT_EOF )
  {
    std::cerr << "Socket closed" << std::endl;
    connection->close();
    connection->getData()->server->onConnectionEvent( connection, ConnectionEvent::Disconnect );
  }

  if ( flags & BEV_EVENT_ERROR )
  {
    std::cerr << "ERROR buffer event: " << EVUTIL_SOCKET_ERROR() << std::endl;
    connection->close();
    connection->getData()->server->onConnectionEvent( connection, ConnectionEvent::Error );
  }

//  if ( flags & BEV_EVENT_READING )
//  {
//  }
//
//  if ( flags & BEV_EVENT_WRITING )
//  {
//  }

  if ( flags & BEV_EVENT_TIMEOUT )
  {
    std::cerr << "Timeout event occured" << std::endl;
    connection->getData()->server->onConnectionEvent( connection, ConnectionEvent::Timeout );
  }

  connectionUpdate( connection );
}


void connectionUpdate( Connection* connection )
{
  if ( connection->_data->close )
  {
    std::cout << "  Closing Connection" << std::endl;
    bufferevent_free( connection->_data->bufferEvent );
    delete connection;
  }
}

