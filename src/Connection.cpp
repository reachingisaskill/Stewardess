
#include "Connection.h"
#include "Serializer.h"
#include "CallbackInterface.h"
#include "EventCallbacks.h"


namespace Stewardess
{

  HugeID Connection::_idCounter = 0;
  std::mutex Connection::_idCounterMutex;


  Connection::Connection( sockaddr address, Manager& manager, event_base* worker_base, evutil_socket_t new_socket ) :
    _references(),
    _idNumber( 0 ),
    _identifier( 0 ),
    _close( false ),
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
      GuardLock lk( _idCounterMutex );
      _idNumber = _idCounter++;
    }

    {
      GuardLock lk( _theMutex );
      _readEvent = event_new( worker_base, new_socket, EV_READ|EV_PERSIST, readCB, this );
      _writeEvent = event_new( worker_base, new_socket, EV_WRITE, writeCB, this );

      event_add( _readEvent, nullptr );
    }

    DEBUG_STREAM( "Stewardess::Connection" ) << "Created connection " << _idNumber;
  }


  Connection::~Connection()
  {
    if ( _readEvent != nullptr )
      event_free( _readEvent );
    if ( _writeEvent != nullptr )
      event_free( _writeEvent );
    if ( serializer != nullptr )
      delete serializer;
    DEBUG_STREAM( "Stewardess::Connection" ) << "Deleted connection " << _idNumber;
  }


  void Connection::close()
  {
    GuardLock lk( _theMutex );
    _close = true;
    event_del( _readEvent );
    event_del( _writeEvent );

    manager.closeConnection( this );
  }


  bool Connection::isOpen()
  {
    GuardLock lk( _theMutex );
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
    if ( _identifier == 0 )
    {
      _identifier = num;
    }
    else
    {
      WARN_STREAM( "Connection::setIdentifier" ) << "Attempting to set Identifier when one already exists for connection : " << _idNumber;
    }
  }


  Handle Connection::requestHandle()
  {
    return Handle( _references, this );
  }


  size_t Connection::getNumberHandles() const
  {
    return _references.getNumber();
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

