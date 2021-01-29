
#ifndef HANDLER_H_
#define HANDLER_H_

#include "Definitions.h"
#include <event2/event.h>


struct Handler
{
  // Control access to this handler
  std::mutex mutex;

  // The actual thread handle
  std::thread theThread;

  // Pointer to the event base for this worker thread
  event_base* eventBase;

  // Pointer to a signal event to allow us to close down safely
  event* timeoutEvent;

  // How long the timeout lasts for 
  timeval timeout;

  // Map of the connections that this thread oversees
  ConnectionMap connections;
};


void handlerThread( Handler* );


#endif // HANDLER_H_

