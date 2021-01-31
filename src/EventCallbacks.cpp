
#include "EventCallbacks.h"
#include "CallbackInterface.h"
#include "ManagerData.h"
#include "Connection.h"
#include "ConnectionData.h"
#include "Handler.h"
#include "Serializer.h"

#include <cmath>


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
  Handler* the_handler = &data->threads[ data->nextThread ]->theHandler;
  if ( (++data->nextThread) == data->threads.size() )
  {
    data->nextThread = 0;
  }

  // Lock the handler while we add a buffer event to the event_base
  UniqueLock lock( the_handler->mutex );


  // Create a buffer event, bound to the tcp socket. When freed it will close the socket.
  bufferevent* buffer_event = bufferevent_socket_new( the_handler->eventBase, new_socket, BEV_OPT_CLOSE_ON_FREE );


  // Create the connection data
  ConnectionData* connectionData = new ConnectionData();
  connectionData->bufferEvent = buffer_event;
  connectionData->server = data->server;
  connectionData->serializer = data->server->buildSerializer();
  connectionData->readBuffer.reserve( data->bufferSize );
  connectionData->writeBuffer.reserve( data->bufferSize );
  connectionData->close = false;

  // Byte-wise copy the address struct
  std::memcpy( (void*)&connectionData->socketAddress, (void*)address, address_length );

    
  // Make a connection object
  Connection* connection = new Connection( connectionData );
  // Add the new connection to the handler
  the_handler->connections[ connection->getIDNumber() ] = connection;
  // Signal that something has connected
  connectionData->server->onConnectionEvent( connection, ConnectionEvent::Connect );

  // Set the call back functions
  bufferevent_setcb( buffer_event, bufferReadCB, bufferWriteCB, bufferEventCB, (void*)connection );

  // Set the time outs for reading & writing only if they're > 0
  timeval* read_timeout = ( data->readTimeout.tv_sec > 0 ) ? &data->readTimeout : nullptr;
  timeval* write_timeout = ( data->writeTimeout.tv_sec > 0 ) ? &data->writeTimeout : nullptr;
  bufferevent_set_timeouts( buffer_event, read_timeout, write_timeout ); 

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
// Server signal callbacks

void interruptSignalCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  ManagerData* data = (ManagerData*)arg;

  std::cerr << "Interrupt received. Time to go." << std::endl;

  // If the manager has the permission to close the connections
  if ( data->connectionCloseOnShutdown )
  {
    ConnectionMap::iterator


    // Set the close flag on the handlers
    for ( ThreadVector::iterator han_it = data->threads.begin(); han_it != data->threads.end(); ++han_it )
    {
      Handler& han = (*han_it)->theHandler;
      // Lock the handler
      GuardLock han_lock( han.mutex );

      // Signal the close flag for each thread handler the threads internal tick event will close the connections
      han.closeConnections = true;
    }
  }

  // Make the death timer pending
  event_add( data->deathEvent, &data->deathTime );

  // Disable the listener and signal handler
  evconnlistener_disable( data->listener );
  evsignal_del( data->signalEvent );

  // Trigger the server call back
  data->server->onEvent( ServerEvent::Shutdown );
}


void killTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  ManagerData* data = (ManagerData*)arg;

  std::cout << "Death timer expired. Time to die." << std::endl;

  for ( ThreadVector::iterator it = data->threads.begin(); it != data->threads.end(); ++it )
  {
    Handler& han = (*it)->theHandler;
    GuardLock lock( han.mutex );
    event_base_loopbreak( han.eventBase );
  }
}


void workerTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  Handler* handler = (Handler*)arg;
  GuardLock( handler->mutex );

  // See which connections are stale/flagged for deletion and remove them
  ConnectionMap::iterator conn_it = handler->connections.begin();
  while ( conn_it != handler->connections.end() )
  {
    ConnectionData* conn_data = conn_it->second->getData();

    if ( conn_data->close || handler->closeConnections )
    {
      // Free the associated buffer event
      bufferevent_free( conn_data->bufferEvent );
      
      // Delete the data
      delete conn_it->second;

      // Remove from the handler map
      conn_it = handler->connections.erase( conn_it );
    }
    else
    {
      ++conn_it;
    }
  }

  // Set the timeout time to the log of the number of connections
  handler->timeout.tv_sec = handler->timeoutModifier * ( 2 + std::log10( handler->connections.size() + 1 ) );
  event_add( handler->timeoutEvent, &handler->timeout);

  std::cout << "TICK " << handler->timeout.tv_sec;
}


////////////////////////////////////////////////////////////////////////////////
// Client signal callbacks

void clientInterruptSignalCB( evutil_socket_t /*socket*/, short /*flags*/, void* arg )
{
  ManagerData* data = (ManagerData*)arg;
  std::cerr << "Interrupt received. Time to go." << std::endl;

  // If the manager has the permission to close the connections
  if ( data->connectionCloseOnShutdown )
  {
  }

  // Make the death timer pending
  event_add( data->deathEvent, &data->deathTime );

  // Disable the signal handler
  evsignal_del( data->signalEvent );

  // Trigger the server call back
  data->server->onEvent( ServerEvent::Shutdown );
}


void clientKillTimerCB( evutil_socket_t /*socket*/, short /*flag*/, void* arg )
{
  ManagerData* data = (ManagerData*)arg;

  std::cout << "Death timer expired. Time to die." << std::endl;
  event_base_loopbreak( data->eventBase );
}


////////////////////////////////////////////////////////////////////////////////
// Buffer event callback functions

void bufferReadCB( bufferevent* buffer_event, void* arg )
{
  Connection* connection = (Connection*)arg;
  ConnectionData* connection_data = connection->getData();

  Serializer* serializer = connection_data->serializer;
  Buffer& buffer = connection_data->readBuffer;
  std::cout << "  Buffer Read called" << std::endl;

  int result;

  evbuffer* read_data = bufferevent_get_input( buffer_event );

  while( connection->isOpen() && ( evbuffer_get_length( read_data ) > 0 ) )
  {
    result = evbuffer_remove( read_data, buffer.data(), buffer.capacity() );

    std::cout << "Read " << result << std::endl;

    if ( result <= 0 )
    {
      buffer.resize( 0 );
      break;
    }

    buffer.resize( result );
    serializer->deserialize( &buffer );

    while( ! serializer->payloadEmpty() )
    {
      std::cout << "Triggering read event." << std::endl;
      connection_data->server->onRead( serializer->getPayload(), connection );
    }

    if ( ! connection->isOpen() ) break;

    while( ! serializer->errorEmpty() )
    {
      std::cerr << "Serializer error occured: " << serializer->getError() << std::endl;
      connection_data->server->onConnectionEvent( connection, ConnectionEvent::Error );
    }

    if ( ! connection->isOpen() ) break;

    while ( ! serializer->bufferEmpty() )
    {
      Buffer* buf = serializer->getBuffer();
      bufferevent_write( buffer_event, buf->data(), buf->size() );
      delete buf;
    }
  }

  std::cout << "  Finished reading" << std::endl;
}


void bufferWriteCB( bufferevent* buffer_event, void* data )
{
  Connection* connection = (Connection*)data;
  ConnectionData* connection_data = connection->getData();

  Serializer* serializer = connection_data->serializer;
  std::cout << "  Buffer Write called" << std::endl;

  connection->getData()->server->onWrite( connection );

  while( ! serializer->errorEmpty() )
  {
    std::cerr << "Serializer error occured: " << serializer->getError() << std::endl;
    connection_data->server->onConnectionEvent( connection, ConnectionEvent::Error );
  }

  while ( ! serializer->bufferEmpty() )
  {
    Buffer* buf = serializer->getBuffer();
    bufferevent_write( buffer_event, buf->data(), buf->size() );
    delete buf;
  }
}


void bufferEventCB( bufferevent* buffer_event, short flags, void* data )
{
  Connection* connection = (Connection*)data;
  ConnectionData* connection_data = connection->getData();
  Serializer* serializer = connection_data->serializer;

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
    if ( flags & BEV_EVENT_READING )
    {
      std::cerr << "Read Timeout event occured" << std::endl;
    }
    else if ( flags & BEV_EVENT_WRITING )
    {
      std::cerr << "Write Timeout event occured" << std::endl;
    }
    connection->getData()->server->onConnectionEvent( connection, ConnectionEvent::Timeout );
  }

  while( ! serializer->errorEmpty() )
  {
    std::cerr << "Serializer error occured: " << serializer->getError() << std::endl;
    connection_data->server->onConnectionEvent( connection, ConnectionEvent::Error );
  }

  while ( ! serializer->bufferEmpty() )
  {
    std::cerr << "Writing to buffer" << std::endl;
    Buffer* buf = serializer->getBuffer();
    bufferevent_write( buffer_event, buf->data(), buf->size() );
    delete buf;
  }
}

