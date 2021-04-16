
#include "InetAddress.h"

#include "LibeventIncludes.h"
#include "Exception.h"


namespace Stewardess
{

  InetAddress::InetAddress( Type t ) :
    _type( t ),
    _port( 0 )
  {
    this->zero();
  }


  InetAddress::InetAddress( const sockaddr* data ) :
    _type(),
    _port()
  {
    this->zero();

    if ( data->sa_family == AF_INET6 )
    {
      _type = IPv6;

      const sockaddr_in6* ip_addr = reinterpret_cast< const sockaddr_in6* >( data );
      const uint8_t* ptr = ip_addr->sin6_addr.s6_addr;
      for ( unsigned int i = 0; i < 16; ++i )
        _addressIPv6[i] = ptr[i];

      _port = ntohs( ip_addr->sin6_port );
    }
    else if ( data->sa_family == AF_INET )
    {
      _type = IPv4;

      const sockaddr_in* ip_addr = reinterpret_cast< const sockaddr_in* >( data );
      const uint8_t* ptr = reinterpret_cast< const uint8_t* >( &ip_addr->sin_addr.s_addr );
      for ( unsigned int i = 0; i < 4; ++i )
        _addressIPv4[i] = ptr[i];

      _port = ntohs( ip_addr->sin_port );
    }
    else
    {
    }
  }


  InetAddress::InetAddress( std::string, Type t ) :
    _type( t ),
    _port()
  {
    this->zero();

    throw Exception( "InetAddress parse string not implemented yet..." );
  }


  InetAddress::InetAddress( uint8_t oct1, uint8_t oct2, uint8_t oct3, uint8_t oct4, uint16_t port ) :
    _type( IPv4 ),
    _port( port )
  {
    this->zero();

    _addressIPv4[0] = oct1;
    _addressIPv4[1] = oct2;
    _addressIPv4[2] = oct3;
    _addressIPv4[3] = oct4;
  }


  void InetAddress::zero()
  {
    // Set everyting in the address union to zero
    for ( unsigned i = 0; i < 16; ++i )
      _addressIPv6[i] = 0;
  }


  std::string InetAddress::getString() const
  {
    char result[41];

    switch( _type )
    {
      case IPv4 :
        {
          const uint8_t* ptr = reinterpret_cast< const uint8_t* >( &_addressIPv4 );
          std::sprintf( result, "%u.%u.%u.%u", ptr[0], ptr[1], ptr[2], ptr[3] );
        }
        break;

      case IPv6 :
        {
          const uint32_t* ptr = reinterpret_cast< const uint32_t* >( &_addressIPv6 );
          std::sprintf( result, "%X:%X:%X:%X:%X:%X:%X:%X", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7] );
        }
        break;
    }

    return std::string( result );
  }


  std::string InetAddress::getStringFull() const
  {
    char result[49];

    switch( _type )
    {
      case IPv4 :
        {
          const uint8_t* ptr = reinterpret_cast< const uint8_t* >( &_addressIPv4 );
          std::sprintf( result, "%u.%u.%u.%u:%u", ptr[0], ptr[1], ptr[2], ptr[3], _port );
        }
        break;

      case IPv6 :
        {
          const uint32_t* ptr = reinterpret_cast< const uint32_t* >( &_addressIPv6 );
          std::sprintf( result, "[%X:%X:%X:%X:%X:%X:%X:%X]:%u", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], _port );
        }
        break;
    }

    return std::string( result );
  }

}

