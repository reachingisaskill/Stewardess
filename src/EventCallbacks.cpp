
#include "EventCallbacks.h"
#include "ServerState.h"
#include "Manager.h"
#include "ManagerData.h"
#include "Connection.h"
#include "ConnectionData.h"
#include "Handler.h"
#include "Serializer.h"

#include <cmath>


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
  Handler* the_handler = data->threads[ data->nextThread ];
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
  connectionData->handler = the_handler;
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

  // Set the time outs for reading & writing
  bufferevent_set_timeouts( buffer_event, &data->timeout, &data->timeout ); 

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
  for ( ThreadVector::iterator it = data->threads.begin(); it != data->threads.end(); ++it )
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

  // Set the timeout time to the log of the number of connections
  handler->timeout.tv_sec = 1 + std::log10( handler->connections.size() + 1 );
  event_add( handler->timeoutEvent, &handler->timeout);
}


void workerKillCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
{
  Handler* handler = (Handler*)arg;

  event_base_loopbreak( handler->eventBase );
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

  while( connection->isOpen() )
  {
    result = bufferevent_read( buffer_event, buffer.data(), buffer.capacity() );

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

  connectionUpdate( connection );
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

    // Remove from the handler map
    connection->_data->handler->connections.erase( connection->_data->handler->connections.find( connection->getIDNumber() ) );

    // Free the associated buffer event
    bufferevent_free( connection->_data->bufferEvent );
    
    // Delete the data
    delete connection;
  }
}

