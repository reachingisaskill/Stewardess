
#define MONEYPENNY_PORT_NUMBER 2345
#define MAX_LINE 16384


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <cassert>
#include <exception>
#include <iostream>


const char* success_string = "Message received!";


struct fd_state
{
  char buffer[MAX_LINE];
  size_t buffer_used;

  size_t n_written;
  size_t write_upto;

  event* read_event;
  event* write_event;
};


void do_read( evutil_socket_t, short, void* );
void do_write( evutil_socket_t, short, void* );
void do_accept( evutil_socket_t, short, void* );


fd_state* allocate_fd_state( event_base*, evutil_socket_t );
void free_fd_state( fd_state* );



int main( int, char** )
{
  std::cout << "Good morning Q. Waiting for messages." << std::endl;


  // Listening port number
  int port_number = MONEYPENNY_PORT_NUMBER;

  // Listener FD
  evutil_socket_t listener;

  // Server address
  sockaddr_in server_address;

  // Event pointers
  event_base* base = event_base_new();
  event* listener_event;

  // Check it succeeded.
  assert( base );

  // Set the server address family
  server_address.sin_family = AF_INET;
  // Set the port number, checking that it is in network byte order.
  server_address.sin_port = htons( port_number );
  // Set the address. Host machine uses the symnbolic value.
  server_address.sin_addr.s_addr = INADDR_ANY;


  // Create the socket
  listener = socket( AF_INET, SOCK_STREAM, 0 );
  // Make it non-blocking
  evutil_make_socket_nonblocking( listener );


  // Bind the socket to the address and check for success
  if ( bind( listener, (sockaddr*)&server_address, sizeof( server_address ) ) < 0 )
  {
    throw std::runtime_error( "Failed to bind socket to address" );
  }

  listen( listener, 16 );

  listener_event = event_new( base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base );

  event_add( listener_event, nullptr );

  event_base_dispatch( base );

  event_free( listener_event );
  event_base_free( base );


  libevent_global_shutdown();
  std::cout << "\nGoodbye Q." << std::endl;

  return 0;
}



void do_read( evutil_socket_t fd, short /*events*/, void* arg )
{
  fd_state* state = (fd_state*)arg;
  char buf[1024];
  int i;
  ssize_t result;

  std::cout << "Read Event" << std::endl;

  while ( true )
  {
    assert( state->write_event );
    result = read( fd, buf, sizeof( buf )-1 );
    if ( result <= 0 )
    {
      break;
    }

    for ( i = 0; i < result; ++i )
    {
      if ( state->buffer_used < sizeof( state->buffer ) )
      {
        state->buffer[state->buffer_used++] = buf[i];
      }
      if ( buf[i] == '\n' || buf[i] == '\0' )
      {
        assert( state->write_event );
        event_add( state->write_event, nullptr );
        state->write_upto = state->buffer_used;
      }
    }

    std::cout << "Received message: " << buf << std::endl;
  }

  event_del( state->read_event );

  if ( result == 0 )
  {
    free_fd_state( state );
  }
  else if ( result < 0 )
  {
    if ( errno == EAGAIN ) 
      return;
    else
    {
      // Oh fuck.
      free_fd_state( state );
    }
  }
}


void do_write( evutil_socket_t fd, short /*events*/, void* arg )
{
  fd_state* state = (fd_state*)arg;
  ssize_t result;

  std::cout << "Write Event" << std::endl;

  while ( state->n_written < state->write_upto )
  {
    result = write( fd, state->buffer + state->n_written, state->write_upto - state->n_written );

    if ( result < 0 )
    {
      if ( errno == EAGAIN ) 
      {
        return;
      }
      else
      {
        free_fd_state( state );
        return;
      }
    }
    assert( result != 0 );
    state->n_written += result;
  }

  if ( state->n_written == state->buffer_used )
  {
    state->n_written = 1;
    state->write_upto = 1;
    state->buffer_used = 1;
  }

  event_del( state->write_event );
  free_fd_state( state );

}


void do_accept( evutil_socket_t listener, short /*events*/, void* arg )
{
  event_base* base = (event_base*)arg;
  sockaddr_storage ss;

  std::cout << "Accept Event" << std::endl;

  socklen_t slen = sizeof( ss );

  int fd = accept( listener, (sockaddr*)&ss, &slen );

  if ( fd < 0 )
  {
    // Bugger!
    return;
  }
  else if ( fd > FD_SETSIZE )
  {
    close( fd );
  }
  else
  {
    evutil_make_socket_nonblocking( fd );

    fd_state* state = allocate_fd_state( base, fd );
    assert( state );

    event_add( state->read_event, nullptr );
  }
}


fd_state* allocate_fd_state( event_base* base, evutil_socket_t fd )
{
  fd_state* state = new fd_state();

  state->read_event = event_new( base, fd, EV_READ|EV_PERSIST, do_read, state );
  state->write_event = event_new( base, fd, EV_WRITE|EV_PERSIST, do_write, state );

  if ( state->read_event == nullptr || state->write_event == nullptr )
  {
    throw std::runtime_error( "Could not create read/write events." );
  }

  state->buffer_used = state->n_written = state->write_upto = 0;

  return state;
}


void free_fd_state( fd_state* state )
{
  event_free( state->read_event );
  event_free( state->write_event );
  delete state;
}


/*
  try
  {
    // Create a socket with an internet address
    socket_fd = socket( AF_INET, SOCK_STREAM, 0 );

    // Check it was successful
    if ( socket_fd < 0 )
    {
      throw std::runtime_error( "Failed to open socket." );
    }

    // Bind the socket to the address and check for success
    if ( bind( socket_fd, (sockaddr*)&server_address, sizeof( server_address ) ) < 0 )
    {
      throw std::runtime_error( "Failed to bind socket to address" );
    }


    // Listen to the socket and wait for message. Queue connections 5 deep. This function cannot fail.
    listen( socket_fd, 5 );

    // Wait for a connection to the client
    new_socket_fd = accept( socket_fd, (sockaddr*)&client_address, &client_address_size );

    // Make sure we succeeded
    if ( new_socket_fd < 0 )
    {
      throw std::runtime_error( "Could not connect to client." );
    }

    // Block until the client has written something. Then read it to the buffer
    result = read( new_socket_fd, buffer, 255 );

    if ( result < 0 )
    {
      throw std::runtime_error( "An error occured while reading from the socket" );
    }


    std::cout << "You have a message Mr. Bond:\n\n" << buffer << std::endl;


    result = write( new_socket_fd, success_string, sizeof( success_string ) );

    if ( result < 0 )
    {
      throw std::runtime_error( "An error occured while writing to the socket" );
    }

    close( new_socket_fd );
    close( socket_fd );
  }
  catch( std::exception& ex )
  {
    std::cerr << "An error occured: " << ex.what() << std::endl;
    return 1;
  }

*/

