
#ifndef STEWARDESS_EVENT_CALLBACKS_H_
#define STEWARDESS_EVENT_CALLBACKS_H_

#include "Definitions.h"
#include "LibeventIncludes.h"


namespace Stewardess
{

  ////////////////////////////////////////////////////////////////////////////////
  // Declare the listener callback functions

  void listenerAcceptCB( evconnlistener*, evutil_socket_t, sockaddr*, int, void* );
  void listenerErrorCB( evconnlistener*, void* );


  ////////////////////////////////////////////////////////////////////////////////
  // Server call back functions

  void startCB( evutil_socket_t, short, void* );
  void interruptSignalCB( evutil_socket_t, short, void* );
  void killTimerCB( evutil_socket_t, short, void* );
  void tickTimerCB( evutil_socket_t, short, void* );
  void userTimerCB( evutil_socket_t, short, void* );
  void connectCB( evutil_socket_t, short, void* );


  ////////////////////////////////////////////////////////////////////////////////
  // Declare the event call back functions

  void readCB( evutil_socket_t, short, void* );
  void writeCB( evutil_socket_t, short, void* );
  void destroyCB( evutil_socket_t, short, void* );

  void workerTickCB( evutil_socket_t, short, void* );

}

#endif // STEWARDESS_EVENT_CALLBACKS_H_

