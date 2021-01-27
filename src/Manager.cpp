
#include "Manager.h"
#include "ManagerData.h"


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////


Manager::Manager( int port_number ) :
  _data( nullptr )
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

  _data = new ManagerData( this, port_number );
}


Manager::~Manager()
{
  if ( _data != nullptr )
    delete _data;

  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
    libevent_global_shutdown();
}


void Manager::run()
{
  // Setup and start the libevent listener loop and return
  std::cout << "Calling dispatch" << std::endl;
  _data->dispatch();
}


void Manager::close()
{
}


bool Manager::pop( Message& message )
{
  return _messageBuffer.waitPop( message );
}


void Manager::publishMessage( Message message )
{
  std::cout << "Publishing message" << std::endl;
  // Push the message to the buffer
  _messageBuffer.push( message );
  // Notify a change on the condition variable in case anyone is waiting
  _bufferCondition.notify_one();
}

