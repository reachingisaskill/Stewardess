
#ifndef STEWARDESS_TIMER_DATA_H__
#define STEWARDESS_TIMER_DATA_H__

#include "Definitions.h"


namespace Stewardess
{

  struct TimerData
  {
    // Pointer to the manager data
    ManagerImpl* manager;

    // Unique ID to distinguish between the timers
    UniqueID timerID;

    // Pointer to the libevent timer event
    event* theEvent;

    // Will be automatically added after the call back has been triggered
    bool repeat;

    // Store the countdown time
    timeval time;
  };

}

#endif // STEWARDESS_TIMER_DATA_H__

