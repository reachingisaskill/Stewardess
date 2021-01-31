
#include "Handler.h"
#include "EventCallbacks.h"
#include "Connection.h"
#include "ConnectionData.h"
#include <csignal>


void handlerThread( Handler* handler )
{
  handler->eventBase = event_base_new();
  if ( handler->eventBase == nullptr )
  {
    throw std::runtime_error( "Could not create a base event. Unknow error." );
  }

  handler->timeout = defaultTickTime;
  handler->timeoutEvent = evtimer_new( handler->eventBase, workerTimerCB, (void*)handler );
  evtimer_add( handler->timeoutEvent, &handler->timeout );


  // Start the libevent loop using the base event
  std::cout << "Configured worker. Dispatching." << std::endl;
  event_base_loop( handler->eventBase, EVLOOP_NO_EXIT_ON_EMPTY );


  // Clear any data from outstanding connections
  for ( ConnectionMap::iterator it = handler->connections.begin(); it != handler->connections.end(); ++it )
  {
    std::cout << "  Closing Connection" << std::endl;

    // Free the associated buffer event
    bufferevent_free( it->second->getData()->bufferEvent );
    
    // Delete the data
    delete it->second;
  }
  handler->connections.clear();

  event_free( handler->timeoutEvent );
  event_base_free( handler->eventBase );
}

