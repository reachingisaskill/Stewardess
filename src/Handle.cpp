
#include "Handle.h"
#include "Connection.h"
#include "Payload.h"
#include "Serializer.h"


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


size_t Handle::getIDNumber() const
{
  return _data->getIdentifier();
}


std::string Handle::getIPAddress() const
{
  // 40 Allows for IPv6 + \0
  char address_string[40];
  evutil_inet_ntop( _data->socketAddress.sin_family, &_data->socketAddress.sin_addr, address_string, 40 );
  return std::string( address_string );
}

int Handle::getPortNumber() const
{
  return _data->socketAddress.sin_port;
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
  _data->serializer->serialize( p );
}


TimeStamp Handle::creationTime() const
{
  return _data->getCreationTime();
}


TimeStamp Handle::lastAccess() const
{
  return _data->getAccess();
}

