
#include "Handler.h"
#include "EventCallbacks.h"
#include <csignal>


#define HANDLER_LOOP_TIME { 1, 0 }


void handlerThread( Handler* handler )
{
  handler->eventBase = event_base_new();
  if ( handler->eventBase == nullptr )
  {
    throw std::runtime_error( "Could not create a base event. Unknow error." );
  }


  // Configure some sort of user event to trigger a closing of connections...

  handler->timeout = HANDLER_LOOP_TIME;
  handler->timeoutEvent = evtimer_new( handler->eventBase, workerTimerCB, (void*)handler );
  evtimer_add( handler->timeoutEvent, &handler->timeout );


  // Start the libevent loop using the base event
  std::cout << "Configured worker. Dispatching." << std::endl;
  event_base_loop( handler->eventBase, EVLOOP_NO_EXIT_ON_EMPTY );


  // Do some cleanup

  event_free( handler->timeoutEvent );
  event_base_free( handler->eventBase );
}

