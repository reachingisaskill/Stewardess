
#include "Handle.h"
#include "Connection.h"
#include "Payload.h"
#include "Serializer.h"


namespace Stewardess
{

  Handle::Handle() :
    _connection( nullptr )
  {
  }


  Handle::Handle( Connection* d ) :
    _connection( d )
  {
    _connection->incrementReferences();
  }


  Handle::~Handle()
  {
    if ( _connection )
    {
      _connection->decrementReferences();
    }
  }


  Handle::Handle( const Handle& other ) :
    _connection( other._connection )
  {
    if ( _connection )
    {
      _connection->incrementReferences();
    }
  }


  Handle::Handle( Handle&& other ) :
    _connection( std::exchange( other._connection, nullptr ) )
  {
  }


  Handle& Handle::operator=( const Handle& other )
  {
    if ( _connection )
    {
      _connection->decrementReferences();
    }

    _connection = other._connection;

    if ( _connection )
    {
      _connection->incrementReferences();
    }

    return *this;
  }


  Handle& Handle::operator=( Handle&& other )
  {
    if ( _connection )
    {
      _connection->decrementReferences();
    }

    _connection = std::exchange( other._connection, nullptr );

    return *this;
  }


  void Handle::release()
  {
    if ( _connection )
    {
      _connection->decrementReferences();
      _connection = nullptr;
    }
  }


  std::string Handle::getIPAddress() const
  {
    if ( _connection->socketAddress.sa_family == AF_INET )
    {
      sockaddr_in* address_pointer = (sockaddr_in*)&_connection->socketAddress;
      // 40 Allows for IPv6 + \0
      char address_string[40];
      evutil_inet_ntop( address_pointer->sin_family, &address_pointer->sin_addr, address_string, 40 );
      return std::string( address_string );
    }
    else
    {
      return std::string( "" );
    }
  }


  int Handle::getPortNumber() const
  {
    if ( _connection->socketAddress.sa_family == AF_INET )
    {
      sockaddr_in* address_pointer = (sockaddr_in*)&_connection->socketAddress;
      return address_pointer->sin_port;
    }
    else
    {
      return 0;
    }
  }


  bool Handle::isOpen() const
  {
    return _connection->isOpen();
  }


  void Handle::close() const
  {
    _connection->close();
  }


  void Handle::write( Payload* p ) const
  {
    _connection->write( p );
  }


  ConnectionID Handle::getConnectionID() const
  {
    return _connection->getConnectionID();
  }


  UniqueID Handle::getIdentifier() const
  {
    return _connection->getIdentifier();
  }


  void Handle::setIdentifier( UniqueID id )
  {
    _connection->setIdentifier( id );
  }


  TimeStamp Handle::creationTime() const
  {
    return _connection->getCreationTime();
  }


  TimeStamp Handle::lastAccess() const
  {
    return _connection->getAccess();
  }

}

