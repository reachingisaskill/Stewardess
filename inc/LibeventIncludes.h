
#ifndef LIBEVENT_INCLUDES_H_
#define LIBEVENT_INCLUDES_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

struct BufferData
{
  evbuffer* data;
};


#endif // LIBEVENT_INCLUDES_H_

