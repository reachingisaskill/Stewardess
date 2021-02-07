
#include "Handle.h"
#include "Connection.h"
#include "Payload.h"
#include "Serializer.h"


namespace Stewardess
{

  Handle::Handle() :
    _data( nullptr ),
    _counter()
  {
  }


  Handle::Handle( ReferenceCounter counter, Connection* d ) :
    _data( d ),
    _counter( counter )
  {
  }


  Handle::~Handle()
  {
  }


  std::string Handle::getIPAddress() const
  {
    if ( _data->socketAddress.sa_family == AF_INET )
    {
      sockaddr_in* address_pointer = (sockaddr_in*)&_data->socketAddress;
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
    if ( _data->socketAddress.sa_family == AF_INET )
    {
      sockaddr_in* address_pointer = (sockaddr_in*)&_data->socketAddress;
      return address_pointer->sin_port;
    }
    else
    {
      return 0;
    }
  }


  bool Handle::isOpen() const
  {
    return _data->isOpen();
  }


  void Handle::close() const
  {
    _data->close();
  }


  void Handle::write( Payload* p ) const
  {
    _data->write( p );
  }


  UniqueID Handle::getIDNumber() const
  {
    return _data->getIDNumber();
  }


  UniqueID Handle::getIdentifier() const
  {
    return _data->getIdentifier();
  }


  void Handle::setIdentifier( UniqueID id )
  {
    _data->setIdentifier( id );
  }


  TimeStamp Handle::creationTime() const
  {
    return _data->getCreationTime();
  }


  TimeStamp Handle::lastAccess() const
  {
    return _data->getAccess();
  }

}

