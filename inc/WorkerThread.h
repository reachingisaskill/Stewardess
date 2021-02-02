
#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "Definitions.h"
#include "LibeventIncludes.h"


struct WorkerData
{
  event_base* eventBase;
  event* tickEvent;
  timeval tickTime;
};


struct ThreadInfo
{
  // The thread object
  std::thread theThread;
  WorkerData data;
};


void workerThread( WorkerData );


#endif // WORKER_THREAD_H_

