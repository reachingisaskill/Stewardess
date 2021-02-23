
#include "Connection.h"
#include "Serializer.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"


namespace Stewardess
{

  Connection::Connection( sockaddr address, Manager& manager, event_base* worker_base, evutil_socket_t new_socket ) :
    _references( 0 ),
    _identifier( 0 ),
    _close( false ),
    _socket( new_socket ),
    _readEvent( nullptr ),
    _writeEvent( nullptr ),
    _connectionTime( std::chrono::system_clock::now() ),
    _lastAccess( std::chrono::system_clock::now() ),
    socketAddress( address ),
    manager( manager ),
    serializer( manager._server.buildSerializer() ),
    bufferSize( 4096 )
  {
    {
      GuardLock lk( _theMutex );
      _readEvent = event_new( worker_base, new_socket, EV_READ|EV_PERSIST, readCB, this );
      _writeEvent = event_new( worker_base, new_socket, EV_WRITE, writeCB, this );

      event_add( _readEvent, nullptr );
    }

    DEBUG_STREAM( "Stewardess::Connection" ) << "Created connection " << this->getConnectionID();
  }


  Connection::~Connection()
  {
    if ( _readEvent != nullptr )
      event_free( _readEvent );
    if ( _writeEvent != nullptr )
      event_free( _writeEvent );
    if ( serializer != nullptr )
      delete serializer;

    DEBUG_STREAM( "Stewardess::Connection" ) << "Deleted connection " << this->getConnectionID();
  }


  void Connection::incrementReferences()
  {
    _references += 1;
  }


  void Connection::decrementReferences()
  {
    _references -= 1;
    if ( _references == 0 )
    {
      if ( _close )
      {
        manager.closeConnection( this );
      }
    }
  }


  void Connection::close()
  {
    DEBUG_STREAM( "Stewardess::Connection" ) << "Close requested " << this->getConnectionID() << " References = " << _references;

    // Lock here in case two threads try to close the connection at the same time!
    UniqueLock lk( _theMutex );
    bool close = _close;
    _close = true;
    lk.unlock();

    if ( ! close )
    {
      event_del( _readEvent );
      event_del( _writeEvent );

      // Damn C libraries and their lack of namespaces....
      ::close( _socket );

      // If no one else cares we suicide.
      if ( _references == 0 )
        manager.closeConnection( this );
    }
  }


  bool Connection::isOpen()
  {
    return !_close;
  }


  void Connection::write( Payload* p )
  {
    GuardLock lk( _theMutex );
    serializer->serialize( p );
    event_add( _writeEvent, nullptr );
  }


  void Connection::setIdentifier( UniqueID num )
  {
    GuardLock lk( _theMutex );
    _identifier = num;
  }


  Handle Connection::requestHandle()
  {
    if ( _close )
      return Handle();
    else
      return Handle( this );
  }


  size_t Connection::getNumberHandles() const
  {
    return _references;
  }


  TimeStamp Connection::getCreationTime() const
  {
    return _connectionTime;
  }


  void Connection::touchAccess()
  {
    GuardLock lk( _lastAccessMutex );
    _lastAccess = std::chrono::system_clock::now();
  }


  TimeStamp Connection::getAccess() const
  {
    GuardLock lk( _lastAccessMutex );
    return _lastAccess;
  }

}

