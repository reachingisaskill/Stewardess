
#ifndef STEWARDESS_WORKER_THREAD_H_
#define STEWARDESS_WORKER_THREAD_H_

#include "Definitions.h"
#include "LibeventIncludes.h"


namespace Stewardess
{

  struct WorkerData
  {
    event_base* eventBase;
    timeval tickTime;
  };


  struct ThreadInfo
  {
    // The thread object
    std::thread theThread;
    WorkerData data;
  };


  void workerThread( WorkerData );

}

#endif // STEWARDESS_WORKER_THREAD_H_

