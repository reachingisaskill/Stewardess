
#ifndef HANDLER_H_
#define HANDLER_H_

#include "Definitions.h"
#include <event2/event.h>


/*
 * This structure stores a reference to all the events assigned to the event_base looping on a given thread
 */
struct Handler
{
  // Pointer to the event base for this worker thread
  event_base* eventBase;

  // Pointer to the timeout event that triggers an internal tick event
  event* timeoutEvent;

  // How long the timeout lasts for 
  timeval timeout;

  // Make the timeout x times longer
  float timeoutModifier;
};


struct ThreadWrapper
{
  std::thread theThread;
  Handler theHandler;
};


void handlerThread( Handler* );


#endif // HANDLER_H_

