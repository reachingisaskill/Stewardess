
#ifndef EVENT_CALLBACKS_H_
#define EVENT_CALLBACKS_H_

#include "LibeventIncludes.h"


////////////////////////////////////////////////////////////////////////////////
// Declare the listener callback functions

void listenerAcceptCB( evconnlistener*, evutil_socket_t, sockaddr*, int, void* );
void listenerErrorCB( evconnlistener*, void* );


////////////////////////////////////////////////////////////////////////////////
// Server call back functions

void interruptSignalCB( evutil_socket_t, short, void* );
void killTimerCB( evutil_socket_t, short, void* );
void tickTimerCB( evutil_socket_t, short, void* );


////////////////////////////////////////////////////////////////////////////////
// Declare the bufferevent call back functions

void bufferReadCB( bufferevent*, void* );
void bufferWriteCB( bufferevent*, void* );
void bufferEventCB( bufferevent*, short, void* );


#endif // EVENT_CALLBACKS_H_

