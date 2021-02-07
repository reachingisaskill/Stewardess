
#include "EventCallbacks.h"
#include "CallbackInterface.h"
#include "Manager.h"
#include "WorkerThread.h"
#include "Handle.h"
#include "Connection.h"
#include "Serializer.h"

#include <cmath>
#include <cstring>
#include <cerrno>


namespace Stewardess
{

  ////////////////////////////////////////////////////////////////////////////////
  // Listener call back functions

  void listenerAcceptCB( evconnlistener* /*listener*/, evutil_socket_t new_socket, sockaddr* address, int /*address_length*/, void* arg )
  {
    Manager* data = (Manager*)arg;
    std::cout << "  Listener Accept Called " << std::endl;

    // Make the socket non-blocking - this happens by default when using a listener
//    evutil_make_socket_nonblocking( new_socket );

    event_base* worker_base;
    // Choose a worker to handle it
    if ( data->singleThreadMode() )
    {
      worker_base = data->_eventBase;
    }
    else
    {
      worker_base = data->_threads[ data->getNextThread() ]->data.eventBase;
    }

    // Create the connection 
    Connection* connection = new Connection( *address, data->_server, worker_base, new_socket );
    connection->readBuffer.reserve( data->_configuration.bufferSize );
      
    // Add the new connection to the manager
    data->addConnection( connection );

    // Signal that something has connected
    connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Connect );
  }


  void listenerErrorCB( evconnlistener* /*listener*/, void* /*data*/ )
  {
  //  Manager* owner = (Manager*)data;
    std::cout << "  Listener Error Called" << std::endl;

    // Handle listener errors

  }


  ////////////////////////////////////////////////////////////////////////////////
  // Server signal callbacks

  void interruptSignalCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    Manager* data = (Manager*)arg;
    std::cerr << "Interrupt received. Time to go." << std::endl;

    // Manager handles the rest.
    data->shutdown();
  }


  void killTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    Manager* data = (Manager*)arg;
    std::cout << "Death timer expired. Time to die." << std::endl;

    // Hard stopping of everything
    data->abort();
  }


  void tickTimerCB( evutil_socket_t /*socket*/, short /*what*/, void* arg )
  {
    Manager* data = (Manager*)arg;

    // If the closed connections have no handles, delete them
    data->cleanupClosedConnections();

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
  // Buffer event callback functions

  void readCB( evutil_socket_t fd, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    Buffer& buffer = connection->readBuffer;
    std::cout << "  Buffer Read called" << std::endl;

    ssize_t result;
    bool good = connection->isOpen();

    while( good )
    {
      buffer.resize( 0 );

      result = read( fd, buffer.data(), buffer.capacity() );
      std::cout << "Read " << result << std::endl;

      if ( result <= 0 )
      {
        if ( result == 0 ) // EOF i.e. socket closed!
        {
          DEBUG_STREAM( "SocketRead" ) << "End of file. Connection: " << connection->getIDNumber();
          connection->close();
          connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Disconnect );
          break;
        }
        else if ( errno == EAGAIN )
        {
          std::cout << "EAGAIN" << std::endl;
          break;
        }
        else
        {
          ERROR_STREAM( "SocketRead" ) << "Connection Error. Connection: " << connection->getIDNumber() << ". Error: " << std::strerror( errno );
          connection->close();
          connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::DisconnectError );
          break;
        }
      }
      else
      {
        std::cout << "Deserialize" << std::endl;
        buffer.resize( result );
        connection->serializer->deserialize( &buffer );
      }
    }

    while ( ! connection->serializer->payloadEmpty() )
    {
      connection->server.onRead( connection->requestHandle(), connection->serializer->getPayload() );
    }

    while( ! connection->serializer->errorEmpty() )
    {
      const char* error = connection->serializer->getError();
      ERROR_STREAM( "SocketRead" ) << "Serializer error occured: " << error;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::SerializationError, error );
    }

    std::cout << "  Buffer Read finished" << std::endl;
    connection->touchAccess();
  }


  void writeCB( evutil_socket_t fd, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    Serializer* serializer = connection->serializer;
    std::cout << "  Buffer Write called" << std::endl;

    ssize_t result;
    ssize_t write_count;
    bool good = connection->isOpen();

    while( ! serializer->errorEmpty() )
    {
      const char* error = connection->serializer->getError();
      ERROR_STREAM( "SocketRead" ) << "Serializer error occured: " << error;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::SerializationError, error );
    }

    while ( ! serializer->bufferEmpty() && good )
    {
      write_count = 0;
      Buffer* buf = serializer->getBuffer();
      while ( good )
      {
        result = write( fd, buf->data() + write_count, buf->size() - write_count );
        std::cout << "Wrote " << result << std::endl;

        if ( result <= 0 )
        {
          if ( result == 0 ) // EOF
          {
            ERROR_LOG( "WriteSocket", "Unexpected end of File" );
            good = false;
            break;
          }
          else if ( errno == EAGAIN )
          {
            DEBUG_STREAM( "WriteSocket" ) << "Connection closed. Connection: " << connection->getIDNumber();
            connection->close();
            connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::DisconnectError );
            good = false;
            break;
          }
          else 
          {
            ERROR_STREAM( "WriteSocket" ) << "An error occured on connection: " << connection->getIDNumber() << ". Error: " << std::strerror( errno );
            connection->close();
            connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::DisconnectError );
            good = false;
            break;
          }
        }
        else if ( result == (ssize_t)buf->size()-write_count ) // Wrote everything
        {
          break;
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
      connection->server.onWrite( connection->requestHandle() );
    }

    std::cout << "  Buffer Write finished" << std::endl;
    connection->touchAccess();
  }

}

