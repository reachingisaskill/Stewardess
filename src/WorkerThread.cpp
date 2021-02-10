
#include "WorkerThread.h"
#include "Exception.h"
#include "EventCallbacks.h"


namespace Stewardess
{

  void workerThread( WorkerData worker_data )
  {
//    // Add a tick event
//    worker_data.tickEvent = evtimer_new( worker_data.eventBase, workerTickCB, (void*)&worker_data );
//    if ( worker_data.tickEvent == nullptr )
//    {
//      throw Exception( "Could not create the worker tick event." );
//    }
//    event_add( worker_data.tickEvent, &worker_data.tickTime );

    // Run the worker loop
    event_base_loop( worker_data.eventBase, EVLOOP_NO_EXIT_ON_EMPTY );

//    // Free the tick event
//    event_free( worker_data.tickEvent );
  }

}

