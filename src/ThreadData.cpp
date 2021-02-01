
#include "ThreadData.h"


ManagerData::ManagerData( Manager& manager ) :
  _manager( manager ),
  _tickTime( { 1, 0 } ),
  eventBase( manager._eventBase ),
  listener( manager._listener ),
  deathEvent( manager._deathEvent ),
  signalEvent( manager._signalEvent ),
  tickEvent( manager._tickEvent ),
  threads( manager._threads ),
  nextThread( 0 ),
  bufferSize( manager._bufferSize ),
  server( manager._server ),
  readTimeout( ( manager._readTimeout.tv_sec > 0 ) ? &manager._readTimeout : nullptr ),
  writeTimeout( ( manager._writeTimeout.tv_sec > 0 ) ? &manager._writeTimeout : nullptr ),
  deathTimeout( &manager._deatTimeout ),
  connectionCloseOnShutdown( manager._connectionCloseOnShutdown )
{
}


void ManagerData::cleanupClosedConnections()
{
  GuardLock lk( = _manager->_closedConnectionsMutex );
  
  ConnectionList::iterator it = _manager->_closedConnections.begin();
  while( it != _manager->_closedConnections.end() )
  {
    if ( (*it)->getNumberHandles() == 0 )
    {
      delete (*it);
      it = _manager->_closedConnections.erase( it );
    }
    else
    {
      ++it;
    }
  }
}


timeval* ManagerData::getTickTime()
{
  GuardLock lk( _manager._connectionsMutex );

  size_t num = _manager._connections.size();

  _tickTime.tv_sec = _manager.configuration.minTickTime + _manager._tickModifier * ( std::log10( num + 1 ) );

  std::cout << "TICK " << _tickTime.tv_sec;

  return & _tickTime;
}


void ManagerData::closeConnection( Connction* connection )
{
  {
    GuardLock lk( _manager._connectionsMutex );
    ConnectionMap::iterator it = _manager._connections.find( connection->getIdentifier() );
    if ( it != _manager._connections.end() )
    {
      _manager._connections.erase( it );
    }
  }

  {
    GuardLock lk( _manager._closedConnections );
    _manager._closedConnections.push_back( connection );
  }
}


void ManagerData::closeAllConnections()
{
  GuardLock lk( _manager._connectionsMutex );

  ConnectionMap::iterator it = _manager._connections.begin();
  while( it != _manager._connections.end() )
  {
    (*it)->close = true;
    this->closeConnection( (*it) );
  }
}


void ManagerData::addConnection( Connection* connection )
{
  _manager->addConnection( connection );
}

