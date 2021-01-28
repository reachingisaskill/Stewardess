
#include "Event.h"
#include "MessageBase.h"
#include <new>


////////////////////////////////////////////////////////////////////////////////
// Individual event member functions

void ReadEvent::free()
{
  delete message;
}


////////////////////////////////////////////////////////////////////////////////
// Union event struct member functions

Event::Event() :
  eventType( EventType::Null ),
  timeStamp( std::time( nullptr ) )
{
}


Event::Event( ListenerEvent e ) :
  eventType( EventType::Listener ),
  timeStamp( std::time( nullptr ) )
{
  new (&listenerEvent) ListenerEvent( e );
}


Event::Event( ReadEvent e ) :
  eventType( EventType::Read ),
  timeStamp( std::time( nullptr ) )
{
  new (&readEvent) ReadEvent( e );
}


Event::Event( WriteEvent e ) :
  eventType( EventType::Write ),
  timeStamp( std::time( nullptr ) )
{
  new (&writeEvent) WriteEvent( e );
}


Event::Event( ConnectionEvent e ) :
  eventType( EventType::Connection ),
  timeStamp( std::time( nullptr ) )
{
  new (&connectionEvent) ConnectionEvent( e );
}


Event::Event( ServerEvent e ) :
  eventType( EventType::Server ),
  timeStamp( std::time( nullptr ) )
{
  new (&serverEvent) ServerEvent( e );
}


Event::Event( const Event& e ) :
  eventType( e.eventType ),
  timeStamp( e.timeStamp )
{
  switch( eventType )
  {
    case EventType::Null :
      break;

    case EventType::Listener :
      new (&listenerEvent) ListenerEvent( e.listenerEvent );
      break;

    case EventType::Read :
      new (&readEvent) ReadEvent( e.readEvent );
      break;

    case EventType::Write :
      new (&writeEvent) WriteEvent( e.writeEvent );
      break;

    case EventType::Connection :
      new (&connectionEvent) ConnectionEvent( e.connectionEvent );
      break;

    case EventType::Server :
      new (&serverEvent) ServerEvent( e.serverEvent );
      break;
  }
}


Event& Event::operator=( const Event& e )
{
  eventType = e.eventType;
  timeStamp = e.timeStamp;

  switch( eventType )
  {
    case EventType::Null :
      break;

    case EventType::Listener :
      listenerEvent.~ListenerEvent();
      new (&listenerEvent) ListenerEvent( e.listenerEvent );
      break;

    case EventType::Read :
      readEvent.~ReadEvent();
      new (&readEvent) ReadEvent( e.readEvent );
      break;

    case EventType::Write :
      writeEvent.~WriteEvent();
      new (&writeEvent) WriteEvent( e.writeEvent );
      break;

    case EventType::Connection :
      connectionEvent.~ConnectionEvent();
      new (&connectionEvent) ConnectionEvent( e.connectionEvent );
      break;

    case EventType::Server :
      serverEvent.~ServerEvent();
      new (&serverEvent) ServerEvent( e.serverEvent );
      break;
  }
  return *this;
}


Event::~Event()
{
  switch( eventType )
  {
    case EventType::Null :
      break;

    case EventType::Listener :
      listenerEvent.~ListenerEvent();
      break;

    case EventType::Read :
      readEvent.~ReadEvent();
      break;

    case EventType::Write :
      writeEvent.~WriteEvent();
      break;

    case EventType::Connection :
      connectionEvent.~ConnectionEvent();
      break;

    case EventType::Server :
      serverEvent.~ServerEvent();
      break;
  }
}


void Event::free()
{
  switch( eventType )
  {
    case EventType::Null :
      break;

    case EventType::Listener :
      listenerEvent.free();
      break;

    case EventType::Read :
      readEvent.free();
      break;

    case EventType::Write :
      writeEvent.free();
      break;

    case EventType::Connection :
      connectionEvent.free();
      break;

    case EventType::Server :
      serverEvent.free();
      break;
  }
}
