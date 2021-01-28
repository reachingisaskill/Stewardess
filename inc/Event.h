
#ifndef EVENT_H_
#define EVENT_H_

#include "Definitions.h"

class Connection;
class MessageBase;


////////////////////////////////////////////////////////////////////////////////

enum class EventType { Null, Listener, Read, Write, Connection, Server };

////////////////////////////////////////////////////////////////////////////////

struct ListenerEvent
{
  Connection* connection;

  void free() {}
};


struct ReadEvent
{
  Connection* connection;
  MessageBase* message;

  void free();
};


struct WriteEvent
{
  Connection* connection;

  void free() {}
};


struct ConnectionEvent
{
  Connection* connection;

  void free() {}
};


struct ServerEvent
{
  std::string errorString;
  void free() {}
};


////////////////////////////////////////////////////////////////////////////////

struct Event
{
  private:
    EventType eventType;
    TimeStamp timeStamp;

  public:

    union
    {
      ListenerEvent listenerEvent;
      ReadEvent readEvent;
      WriteEvent writeEvent;
      ConnectionEvent connectionEvent;
      ServerEvent serverEvent;
    };

    Event();

    Event( ListenerEvent );
    Event( ReadEvent );
    Event( WriteEvent );
    Event( ConnectionEvent );
    Event( ServerEvent );

    Event( const Event& );

    Event& operator=( const Event& );

    ~Event();


    EventType getEventType() { return eventType; }
    
    TimeStamp getTimeStamp() { return timeStamp; }

    void free();
};

#endif // EVENT_H_

