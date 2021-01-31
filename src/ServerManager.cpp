
#include "ServerManager.h"
#include "ManagerData.h"
#include "Handler.h"
#include "EventCallbacks.h"
#include "CallbackInterface.h"

#include <csignal>


////////////////////////////////////////////////////////////////////////////////
// ServerManager Member Functions

ServerManager::ServerManager( int port_number ) :
  ManagerBase()
{
  _data->portNumber = port_number;
}


ServerManager::~ServerManager()
{
}


void ServerManager::_run( CallbackInterface& server )
{
  // Set the server pointer
  _data->server = &server;

  // Configure the socket details
  _data->socketAddress.sin_family = AF_INET;
  _data->socketAddress.sin_addr.s_addr = INADDR_ANY;
  _data->socketAddress.sin_port = htons( _data->portNumber );

  // Configure the event base.
  std::cout << "Creating event_base" << std::endl;
  _data->eventBase = event_base_new();
  if ( _data->eventBase == nullptr )
  {
    throw std::runtime_error( "Could not create a base event. Unknow error." );
  }

  // Build a listener and bind it to a new socket
  std::cout << "Creating listener" << std::endl;
  _data->listener = evconnlistener_new_bind( _data->eventBase, listenerAcceptCB, (void*)_data, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                             (sockaddr*)&_data->socketAddress, sizeof(_data->socketAddress) );
  if ( _data->listener == nullptr )
  {
    throw std::runtime_error( "Could not bind a listener to the requested socket." );
  }

  char address_string[16];
  evutil_inet_ntop( _data->socketAddress.sin_family, &_data->socketAddress.sin_addr, address_string, 16 );
  std::cout << "Bound server to socket. IP Address : " << address_string << std::endl;

  // Create an event to force shutdown, but don't enable it
  _data->deathEvent = evtimer_new( _data->eventBase, killTimerCB, (void*)_data );
  if ( _data->deathEvent == nullptr )
  {
    throw std::runtime_error( "Could not create the death event to the requested socket." );
  }

  // Set the error call back function on the listener
  evconnlistener_set_error_cb( _data->listener, listenerErrorCB );

  // Create a signal event so we can close the system down
  std::cout << "Creating signal event" << std::endl;
  _data->signalEvent = evsignal_new( _data->eventBase, SIGINT, interruptSignalCB, (void*)_data );
  event_add( _data->signalEvent, nullptr );

  // Create a handler for each requested thread
  std::cout << "Intialising workers." << std::endl;
  for ( unsigned int i = 0; i < _data->numThreads; ++i )
  {
    ThreadWrapper* wrapper = new ThreadWrapper();
    wrapper->theHandler.eventBase = nullptr;
    wrapper->theHandler.timeoutEvent = nullptr;
    wrapper->theHandler.timeoutModifier = _data->tickTimeModifier;
    wrapper->theThread = std::thread( handlerThread, &wrapper->theHandler );
    _data->threads.push_back( wrapper );
  }


  // Start the libevent loop using the base event
  std::cout << "Dispatching lister event" << std::endl;
  event_base_dispatch( _data->eventBase );


  // Do some cleanup

  std::cout << "Joining worker threads" << std::endl;
  for ( ThreadVector::iterator it = _data->threads.begin(); it != _data->threads.end(); ++it )
  {
    (*it)->theThread.join();
    delete (*it);
  }

  std::cout << "Freeing events" << std::endl;
  event_free( _data->deathEvent );
  event_free( _data->signalEvent );
  evconnlistener_free( _data->listener );
  event_base_free( _data->eventBase );
}

