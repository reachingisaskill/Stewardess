
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

    evutil_make_socket_nonblocking( new_socket );

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
    bool good = true;

    while( good )
    {
      buffer.resize( 0 );

      result = read( fd, buffer.data(), buffer.capacity() );
      std::cout << "Read " << result << std::endl;

      if ( result <= 0 )
      {
        if ( errno == EAGAIN )
        {
          break;
        }
        else if ( result == 0 ) // EOF i.e. socket closed!
        {
          connection->close();
          break;
        }
        else
        {
          connection->close();
          good = false;
        }
      }
      else
      {
        buffer.resize( result );
        connection->serializer->deserialize( &buffer );
      }
    }

    if ( good )
    {
      while ( ! connection->serializer->payloadEmpty() )
      {
        connection->server.onRead( connection->requestHandle(), connection->serializer->getPayload() );
      }
    }
    else
    {
      std::cerr << "Read error occured: " << std::strerror( errno ) << std::endl;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Error );
    }

    while( ! connection->serializer->errorEmpty() )
    {
      std::cerr << "Serializer error occured: " << connection->serializer->getError() << std::endl;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Error );
    }

    std::cout << "  Finished reading" << std::endl;
    connection->touchAccess();
  }


  void writeCB( evutil_socket_t fd, short /*flags*/, void* arg )
  {
    Connection* connection = (Connection*)arg;
    Serializer* serializer = connection->serializer;
    std::cout << "  Buffer Write called" << std::endl;

    ssize_t result;
    ssize_t write_count;
    bool good = true;


    while( ! serializer->errorEmpty() )
    {
      std::cerr << "Serializer error occured: " << serializer->getError() << std::endl;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Error );
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
          if ( errno == EAGAIN ) break;
          else if ( result == 0 ) // EOF
          {
            good = false;
            break;
          }
          else 
          {
            connection->close();
            good = false;
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

    if ( ! good )
    {
      std::cerr << "Write error occured " << std::strerror( errno ) << std::endl;
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Error );
      connection->server.onConnectionEvent( connection->requestHandle(), ConnectionEvent::Disconnect );
    }
    else
    {
      connection->server.onWrite( connection->requestHandle() );
      connection->touchAccess();
    }

  }

}

