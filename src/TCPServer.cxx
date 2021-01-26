
#define MONEYPENNY_PORT_NUMBER 2345
#define MAX_LINE 16384


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <cassert>
#include <exception>
#include <iostream>


void read_cb( bufferevent* bev, void* ctx );
void event_cb( bufferevent* bev, short events, void* ctx );
void accept_cb( evconnlistener* listener, evutil_socket_t fd, sockaddr* addr, int addrlen, void* ctx );
void error_cb( evconnlistener* bev, void* ctx );



int main( int, char** )
{
  std::cout << "Good morning Q. Waiting for messages." << std::endl;

  // Listening port number
  int port_number = MONEYPENNY_PORT_NUMBER;

  // Listener event
  evconnlistener* listener;

  // Socket address
  sockaddr_in address;

  // Event base pointer
  event_base* base = event_base_new();

  if ( base == nullptr )
  {
    std::cerr << "Could not create event base" << std::endl;
    return 1;
  }

  // Set the server address family
  address.sin_family = AF_INET;
  // Set the port number, checking that it is in network byte order.
  address.sin_port = htons( port_number );
  // Set the address. Host machine uses the symnbolic value.
  address.sin_addr.s_addr = INADDR_ANY;


  listener = evconnlistener_new_bind( base, accept_cb, nullptr, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (sockaddr*)&address, sizeof(address) );

  if ( listener == nullptr )
  {
    std::cerr << "Could not create listener" << std::endl;
    return 1;
  }

  evconnlistener_set_error_cb( listener, error_cb );

  event_base_dispatch( base );


  libevent_global_shutdown();
  std::cout << "\nGoodbye Q." << std::endl;

  return 0;
}


void read_cb( bufferevent* bev, void* ctx )
{
  evbuffer* input = bufferevent_get_input( bev );
  evbuffer* output = bufferevent_get_output( bev );

  evbuffer_add_buffer( output, input );
}


void event_cb( bufferevent* bev, short events, void* ctx )
{
  if ( events & BEV_EVENT_ERROR )
    std::cerr << "Error from buffer event: " << EVUTIL_SOCKET_ERROR() << std::endl;

  if ( events & BEV_EVENT_EOF | BEV_EVENT_ERROR )
    bufferevent_free( bev );
}


void accept_cb( evconnlistener* listener, evutil_socket_t fd, sockaddr* addr, int addrlen, void* ctx )
{
  event_base* base = evconnlistener_get_base( listener );
  bufferevent* bev = bufferevent_socket_new( base, fd, BEV_OPT_CLOSE_ON_FREE );

  bufferevent_setcb( bev, read_cb, nullptr, event_cb, nullptr );
  bufferevent_enable( bev, EV_READ|EV_WRITE );
}


void error_cb( evconnlistener* bev, void* ctx )
{
  event_base* base = evconnlistener_get_base( bev );

  int err = EVUTIL_SOCKET_ERROR();

  std::cerr << "Listener error occured : " << err;

  event_base_loopexit( base, nullptr );
}

