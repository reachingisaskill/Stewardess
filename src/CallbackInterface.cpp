
#include "CallbackInterface.h"
#include "Connection.h"


CallbackInterface::CallbackInterface() :
  _managerData( nullptr )
{
}


CallbackInterface::~CallbackInterface()
{
}


Connection* requestConnection( size_t index )
{
  GuardLock lock( _managerData->connectionMapMutex );

  ConnectionMap::iterator found = _managerData->connections.find( index );
  if ( found == _managerData->connections.end () )
  {
    return nullptr;
  }
  else
  {
    return found->second;
  }
}


size_t getNumberConnections() const
{
  GuardLock lock( _managerData->connectionMapMutex );

  return _managerData->connections.size();
}

