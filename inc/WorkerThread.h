
#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "Definitions.h"


struct WorkerData
{
  event_base* eventBase;
};


struct ThreadInfo
{
  // The thread object
  std::thread theThread;
  WorkerData data;
};


void workerThread( WorkerData& );


#endif // WORKER_THREAD_H_

