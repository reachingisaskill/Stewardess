
#include "WorkerThread.h"
#include "EventCallbacks.h"
#include "Connection.h"
#include "ConnectionData.h"


void handlerThread( WorkerData& worker_data );
{
  // Start the libevent loop using the base event
  std::cout << "Configured worker. Dispatching." << std::endl;

  event_base_loop( worker_data.eventBase, EVLOOP_NO_EXIT_ON_EMPTY );
}

