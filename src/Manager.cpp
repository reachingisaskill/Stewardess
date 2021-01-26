
#include "Manager.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Management data 

class Management
{
  public :
    Management( int ) {}
};


////////////////////////////////////////////////////////////////////////////////

Manager::Manager( int port_number ) :
  _management( nullptr )
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

//  _management = new Management( port_number );
}


Manager::~Manager()
{
  if ( _management != nullptr )
    delete _management;

  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
    libevent_global_shutdown();
}



