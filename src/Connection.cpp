
#include "Connection.h"
#include "Serializer.h"


namespace Stewardess
{

  UniqueID Connection::_idCounter;
  std::mutex Connection::_idCounterMutex;


  Connection::Connection() :
    _references(),
    _identifier( 0 ),
    _close( false ),
    _connectionTime( std::chrono::system_clock::now() ),
    _lastAccess( std::chrono::system_clock::now() ),
    bufferEvent( nullptr ),
    socketAddress(),
    server( nullptr ),
    serializer( nullptr ),
    readBuffer()
  {
    GuardLock lk( _idCounterMutex );
    _identifier = _idCounter++;
  }


  Connection::~Connection()
  {
    if ( bufferEvent != nullptr )
      bufferevent_free( bufferEvent );
    if ( serializer != nullptr )
      delete serializer;
  }


  void Connection::close()
  {
    GuardLock lk( _closeMutex );
    _close = false;
  }


  bool Connection::isOpen()
  {
    GuardLock lk( _closeMutex );
    return !_close;
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

