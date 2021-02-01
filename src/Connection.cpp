
#include "Connection.h"


UniqueID Connection::_idCounter;
std::mutex _idCounterMutex;


Connection::Connection() :
  _references(),
  _identifier( 0 ),
  _close( false ),
  bufferEvent( nullptr ),
  socketAddress(),
  server( nullptr ),
  serializer( nullptr ),
  readBuffer()
{
  GuardLock lk( _idCounterMutex );
  _identifier = _idCounter++;
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



