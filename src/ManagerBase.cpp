
#include "ManagerBase.h"
#include "ManagerData.h"
#include "CallbackInterface.h"


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t ManagerBase::_instanceCount = 0;
std::mutex ManagerBase::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Member functions definitions

ManagerBase::ManagerBase() :
  _data( new ManagerData() )
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

  _data->owner = this;
  _data->server = nullptr;
  _data->portNumber = 0;
  _data->numThreads = 2;
  _data->nextThread = 0;
  _data->bufferSize = 4096;
  _data->tickTimeModifier = 1.0;
  _data->readTimeout = { 3, 0 };
  _data->writeTimeout = { 3, 0 };
  _data->deathTime = { 5, 0 };
  _data->threads.reserve( _data->numThreads );

  std::memset( &_data->socketAddress, 0, sizeof( _data->socketAddress ) );
}


ManagerBase::~ManagerBase()
{
  if ( _data != nullptr )
    delete _data;

  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
  {
    std::cout << "No more managers. Global libevent shutdown." << std::endl;
    libevent_global_shutdown();
  }
}


void ManagerBase::setNumberThreads( unsigned n )
{
  if ( n == 0 )
  {
    std::cerr << "Number of threads must be at least 1" << std::endl;
    _data->numThreads = 1;
  }
  else
  {
    _data->numThreads = n;
  }
}


void ManagerBase::setDefaultBufferSize( size_t buffer_size )
{
  _data->bufferSize = buffer_size;
}


void ManagerBase::setReadTimeout( unsigned int sec )
{
  _data->readTimeout.tv_sec = sec;
}


void ManagerBase::setWriteTimeout( unsigned int sec )
{
  _data->writeTimeout.tv_sec = sec;
}


void ManagerBase::setTickTimeModifier( float m )
{
  if ( m < 1.0E-6 )
  {
    throw std::runtime_error( "Tick time modifier cannot be small or negative." );
  }
  _data->tickTimeModifier = m;
}


void ManagerBase::setDeathTime( unsigned int time )
{
  _data->deathTime.tv_sec = time;
}


void ManagerBase::setCloseConnectionsOnShutdown( bool value )
{
  _data->connectionCloseOnShutdown = value;
}


CallbackInterface& ManagerBase::getCallbackInterface() const
{
  if ( _data->server == nullptr )
  {
    throw std::runtime_error( "Server is not running. Cannot access server state." );
  }
  else
  {
    return *_data->server;
  }
}


void ManagerBase::run( CallbackInterface& server )
{
  server._managerData = _data;
  this->_run( server );
  server._managerData = nullptr;
}

