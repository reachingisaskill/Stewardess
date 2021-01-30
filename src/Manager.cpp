
#include "Manager.h"
#include "ManagerData.h"
#include "Handler.h"
#include "EventCallbacks.h"
#include "ServerState.h"

#include <csignal>


////////////////////////////////////////////////////////////////////////////////
// Static variables

size_t Manager::_instanceCount = 0;
std::mutex Manager::_instanceCountMutex;


////////////////////////////////////////////////////////////////////////////////
// Manager Member Functions

Manager::Manager( int port_number ) :
  _data( nullptr )
{
  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  _instanceCount += 1;

  _data = new ManagerData();
  _data->owner = this;
  _data->server = nullptr;
  _data->numThreads = 2;
  _data->nextThread = 0;
  _data->bufferSize = 4096;
  _data->threads.reserve( _data->numThreads );

  std::memset( &_data->socketAddress, 0, sizeof( _data->socketAddress ) );
  _data->socketAddress.sin_family = AF_INET;
  _data->socketAddress.sin_addr.s_addr = INADDR_ANY;
  _data->socketAddress.sin_port = htons( port_number );
}


Manager::~Manager()
{
  if ( _data != nullptr )
    delete _data;

  std::lock_guard<std::mutex> lock( _instanceCountMutex );
  if ( (--_instanceCount) == 0 )
  {
    std::cout << "No more managers. Global libevent shutdown." << std::endl;
    libevent_global_shutdown();
  }
}


void Manager::setNumberThreads( unsigned n )
{
  if ( n == 0 )
  {
    std::cerr << "Number of threads must be at least 1" << std::endl;
    _data->numThreads = 1;
  }
  else
  {
    _data->numThreads = n;
  }
}


void Manager::run( ServerState& server )
{
  // Set the server pointer
  _data->server = &server;

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
    Handler* new_handler = new Handler();
    new_handler->theThread = std::thread( handlerThread, new_handler );
    _data->threads.push_back( new_handler );
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
  event_free( _data->signalEvent );
  evconnlistener_free( _data->listener );
  event_base_free( _data->eventBase );
}


ServerState& Manager::getServerState() const
{
  if ( _data->server == nullptr )
  {
    throw std::runtime_error( "Server is not running. Cannot access server state." );
  }
  else
  {
    return *_data->server;
  }
}

