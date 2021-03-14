
#include "EventCallbacks.h"
#include "CallbackInterface.h"
#include "ManagerImpl.h"
#include "WorkerThread.h"
#include "Handle.h"
#include "Connection.h"
#include "Serializer.h"
#include "Buffer.h"

#include <cmath>
#include <cstring>
#include <cerrno>


namespace Stewardess
{

  ////////////////////////////////////////////////////////////////////////////////
  // Listener call back functions

  void listenerAcceptCB( evconnlistener* /*listener*/, evutil_socket_t new_socket, sockaddr* address, int /*address_length*/, void* arg )
  {
    ManagerImpl* data = (ManagerImpl*)arg;
    DEBUG_LOG( "Stewardess::Listener", "New connection found" );

    // Make the socket non-blocking - this happens by default when using a listener
//    evutil_make_socket_nonblocking( new_socket );

    event_base* worker_base;
    // Choose a worker to handle it
    if ( data->_threads.size() == 0 )
    {
      worker_base = data->_eventBase;
    }
    else
    {
      worker_base = data->_threads[ data->getNextThread() ]->data.eventBase;
    }

    // Create the connection 
    Connection* connection = new Connection( *address, *data, worker_base, new_socket );
    connection->bufferSize = data->_configuration.bufferSize;
      
    // Add the new connection to the manager
    data->addConnection( connection );

    // Signal that something has connected
    data->_server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Connect );
  }


  void listenerErrorCB( evconnlistener* /*listener*/, void* arg )
  {
    ManagerImpl* data = (ManagerImpl*)arg;

    int err = EVUTIL_SOCKET_ERROR();
    ERROR_STREAM( "Stewardess::Listener" ) << "An error occured with the libevent listener: " << evutil_socket_error_to_string( err );

    data->_server.onEvent( ServerEvent::ListenerError, evutil_socket_error_to_string( err ) );
  }


  ////////////////////////////////////////////////////////////////////////////////
  // Server signal callbacks

  void interruptSignalCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    ManagerImpl* data = (ManagerImpl*)arg;
    INFO_LOG( "Stewardess::SignalHandler", "Interrupt sisgnal received." );

    // ManagerImpl handles the rest.
    data->shutdown();
  }


  void killTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    ManagerImpl* data = (ManagerImpl*)arg;
    INFO_LOG( "Stewardess::SignalHandler", "Shutdown timer expired." );

    // Hard stopping of everything
    data->abort();
  }


  void tickTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    ManagerImpl* data = (ManagerImpl*)arg;

//    // If the closed connections have no handles, delete them
//    data->cleanupClosedConnections();

    // Update the tick time stamp
    TimeStamp new_stamp = std::chrono::system_clock::now();
    auto duration = new_stamp - data->_tickTimeStamp;
    data->_tickTimeStamp = new_stamp;

    // Trigger the callback
    data->_server.onTick( std::chrono::duration_cast<std::chrono::milliseconds>( duration ) );

    // Set the timeout time to the log of the number of connections
    event_add( data->_tickEvent, data->getTickTime() );
  }


  ////////////////////////////////////////////////////////////////////////////////
  // Read/write event callback functions

  void readCB( evutil_socket_t fd, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    DEBUG_LOG( "Stewardess::SocketRead", "Socket Read called" );

    // Keep hold of a handle before anything happens
    Handle temp_handle = connection->requestHandle();

    ssize_t result;
    bool good = connection->isOpen() && temp_handle;

    Buffer buffer;

    while( good )
    {
      char* raw_buffer = new char[ connection->bufferSize ];
      DEBUG_LOG( "Stewardess::SocketRead", "Reading from socket" );
      result = read( fd, raw_buffer, connection->bufferSize );
      DEBUG_STREAM( "Stewardess::SocketRead" ) << "Read " << result;

      if ( result <= 0 )
      {
        if ( result == 0 ) // EOF i.e. socket closed!
        {
          DEBUG_STREAM( "Stewardess::SocketRead" ) << "End of file. Connection: " << connection->getConnectionID();
          connection->close();
          connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::Disconnect );
          delete[] raw_buffer;
          break;
        }
        else if ( errno == EAGAIN )
        {
          DEBUG_STREAM( "Stewardess::SocketRead" ) << "EAGAIN";
          delete[] raw_buffer;
          break;
        }
        else
        {
          ERROR_STREAM( "Stewardess::SocketRead" ) << "Connection Error. Connection: " << connection->getConnectionID() << ". Error: " << std::strerror( errno );
          connection->close();
          connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::DisconnectError );
          delete[] raw_buffer;
          break;
        }
      }
      else
      {
        DEBUG_LOG( "Stewardess::SocketRead", "Deserializing" );
        buffer.pushChunk( raw_buffer, result );
      }
    }

    if ( buffer )
    {
      connection->serializer->deserialize( &buffer );
    }

    while ( ! connection->serializer->payloadEmpty() )
    {
      DEBUG_LOG( "Stewardess::SocketRead", "Calling on read handler" );
      connection->manager._server.onRead( temp_handle, connection->serializer->getPayload() );
    }

    while( ! connection->serializer->errorEmpty() )
    {
      const char* error = connection->serializer->getError();
      ERROR_STREAM( "Stewardess::SocketRead" ) << "Serializer error occured: " << error;
      connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::SerializationError, error );
    }

    DEBUG_LOG( "Stewardess::SocketRead", "Socket Read Finished" );
    connection->touchAccess();
  }


  void writeCB( evutil_socket_t fd, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    Serializer* serializer = connection->serializer;
    DEBUG_LOG( "Stewardess::SocketWrite", "Socket Write Called" );

    // Keep hold of a handle before anything happens
    Handle temp_handle = connection->requestHandle();

    ssize_t result;
    ssize_t write_count;
    bool good = connection->isOpen() && temp_handle;

    while( ! serializer->errorEmpty() )
    {
      const char* error = connection->serializer->getError();
      ERROR_STREAM( "Stewardess::SocketWrite" ) << "Serializer error occured: " << error;
      connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::SerializationError, error );
    }

    while ( ! serializer->bufferEmpty() && good )
    {
      write_count = 0;
      Buffer* buf = serializer->getBuffer();
      while ( *buf )
      {
        result = write( fd, buf->chunk() + write_count, buf->chunkSize() - write_count );
        DEBUG_STREAM( "Stewardess::SocketWrite" ) << "Wrote " << result;

        if ( result <= 0 )
        {
          if ( result == 0 ) // EOF
          {
            ERROR_LOG( "Stewardess::WriteSocket", "Unexpected end of File" );
            good = false;
            break;
          }
          else if ( errno == EAGAIN )
          {
            WARN_STREAM( "Stewardess::SocketWrite" ) << "Connection closed during write operation: " << connection->getConnectionID();
            connection->close();
            connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::DisconnectError );
            good = false;
            break;
          }
          else 
          {
            ERROR_STREAM( "Stewardess::WriteSocket" ) << "An error occured on connection: " << connection->getConnectionID() << ". Error: " << std::strerror( errno );
            connection->close();
            connection->manager._server.onConnectionEvent( temp_handle, ConnectionEvent::DisconnectError );
            good = false;
            break;
          }
        }
        else if ( result == (ssize_t)buf->chunkSize()-write_count ) // Wrote everything
        {
          buf->popChunk();
          write_count = 0;
        }
        else
        {
          write_count += result;
        }
      }
      delete buf;
    }

    if ( good )
    {
      DEBUG_LOG( "Stewardess::SocketWrite", "Calling on write handler" );
      connection->manager._server.onWrite( temp_handle );
    }

    DEBUG_LOG( "Stewardess::SocketWrite", "Socket Write Finished" );
    connection->touchAccess();
  }


  void destroyCB( evutil_socket_t /*fd*/, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    DEBUG_LOG( "Stewardess::SocketClose", "Socket Close Called" );

    connection->manager.closeConnection( connection );
  }


  void workerTickCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    WorkerData* data = (WorkerData*)arg;

    // Re trigger the event
    event_add( data->tickEvent, &data->tickTime );
  }

}

