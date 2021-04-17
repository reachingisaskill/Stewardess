
#ifndef STEWARDESS_INET_ADDRESS_H_
#define STEWARDESS_INET_ADDRESS_H_

#include "Definitions.h"


// Forward declare the sockaddr struct
struct sockaddr;


namespace Stewardess
{

  class InetAddress
  {
      friend bool operator==( const InetAddress&, const InetAddress& );
    public:
      enum Type { IPv4, IPv6 };

    private:
      // Type of address (v4 or v6)
      Type _type;

      // Raw port number in host byte order
      uint16_t _port;

      // Raw address in host byte order
      union
      {
        uint8_t _addressIPv4[4];
        uint8_t _addressIPv6[16];
      };


      void zero();

    public:
      // Create zeroed
      InetAddress( Type = IPv4 );

      // Copy raw data from sockaddr struct
      InetAddress( const sockaddr* );

      // Create from a formatted string and a IPv6 flag
      InetAddress( std::string, Type = IPv4 );

      // Create from a ip address described in octets and a port number
      InetAddress( uint8_t, uint8_t, uint8_t, uint8_t, uint16_t );


      // Return the type of address
      Type getType() const { return _type; }


      // Convert to a string
      std::string getString() const;

      // Convert to a string with port number
      std::string getStringFull() const;


      // Get the port number
      uint16_t getPort() const { return _port; }
  };


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Friend functions

  inline bool operator==( const InetAddress& first, const InetAddress& second )
  {
    if ( first._type != second._type ) return false;

    if ( first._port != second._port ) return false;

    switch( first._type )
    {
      case InetAddress::IPv4 :
        for ( unsigned int i = 0; i < 4; ++i )
        {
          if ( first._addressIPv4[i] != second._addressIPv4[i] ) return false;
        }
        break;


      case InetAddress::IPv6 :
        for ( unsigned int i = 0; i < 16; ++i )
        {
          if ( first._addressIPv6[i] != second._addressIPv6[i] ) return false;
        }
        break;
    }

    return true;
  }


  inline bool operator!=( const InetAddress& first, const InetAddress& second )
  {
    return ! ( first == second );
  }
}

#endif // STEWARDESS_INET_ADDRESS_H_

