
#include "Manager.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"
#include "WorkerThread.h"
#include "Connection.h"
#include "Exception.h"

#include <signal.h>
#include <cstring>
#include <cmath>


namespace Stewardess
{

  ////////////////////////////////////////////////////////////////////////////////
  // Static variables

  size_t Manager::_instanceCount = 0;
  std::mutex Manager::_instanceCountMutex;


  ////////////////////////////////////////////////////////////////////////////////
  // Member functions definitions

  Manager::Manager( const Configuration& config, CallbackInterface& server ) :
    _impl( new ManagerImpl( config._data, server ) )
  {
    std::lock_guard<std::mutex> lock( _instanceCountMutex );
    if ( _instanceCount == 0 )
    {
      if ( evthread_use_pthreads() != 0 )
        ERROR_LOG( "Stewardess::Manager", "Could not enable pthreads?!" );

      // Try to set the sigpipe to ignore. We have non-blocking sockets and handle the return errors.
      sighandler_t old_handler = signal( SIGPIPE, SIG_IGN );
      // If the user set a specific handler, restore that one.
      if ( old_handler != SIG_DFL )
        signal( SIGPIPE, old_handler );
    }
    _instanceCount += 1;

    // Set the manager pointer for the server logic we're using
    server._manager = this;
  }


  Manager::~Manager()
  {
    // Delete the implementation class
    delete _impl;

    std::lock_guard<std::mutex> lock( _instanceCountMutex );
    if ( (--_instanceCount) == 0 )
    {
      libevent_global_shutdown();
    }
  }


  int Manager::getPortNumber() const
  {
    return _impl->getPortNumber();
  }


  std::string Manager::getIPAddress() const
  {
    return _impl->getIPAddress();
  }

  bool Manager::singleThreadMode() const
  {
    return _impl->_threads.size() == 0;
  }


  TimeStamp Manager::getLastTickTime() const
  {
    return _impl->_tickTimeStamp;
  }


  TimeStamp Manager::getStartTime() const
  {
    return _impl->_serverStartTime;
  }


  Seconds Manager::getUpTime() const
  {
    return std::chrono::duration_cast<Seconds>( std::chrono::system_clock::now() - _impl->_serverStartTime );
  }


  void Manager::requestConnectTo( std::string /*host*/, std::string /*port*/, UniqueID /*id*/ )
  {
    // Not yet implemented
    // _impl->requestConnectTo( host, port, id );
  }


  size_t Manager::getNumberConnections() const
  {
    return _impl->getNumberConnections();
  }


  void Manager::run()
  {
    _impl->run();
  }


  void Manager::shutdown()
  {
    _impl->shutdown();
  }


  void Manager::abort()
  {
    _impl->abort();
  }


  Handle Manager::connectTo( std::string host, std::string port, UniqueID id )
  {
    return _impl->connectTo( host, port, id );
  }

}

