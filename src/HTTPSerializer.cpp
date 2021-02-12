
#include "HTTPSerializer.h"
#include "Buffer.h"

#include <cstring>


namespace Stewardess
{

  const char* const MethodStrings[] = { "RESPONSE", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "OPTIONS", "CONNECT", "PATCH" };

  const char* const VersionString = "HTTP/1.1";


  HTTPPayload::MethodType getMethodFromName( std::string );

  std::string getStringFromResponse( HTTPPayload::ResponseType );


////////////////////////////////////////////////////////////////////////////////////////////////////


  static const char* ErrorMalformedPayload = "Malformed payload";


////////////////////////////////////////////////////////////////////////////////////////////////////


  void HTTPSerializer::serialize( const Payload* p )
  {
    DEBUG_LOG( "Stewardess::HTTPSerializer", "Serializing" );

    HTTPPayload* payload = (HTTPPayload*)p;
    std::string request_string;
    std::string header_string;

    if ( payload->_method == HTTPPayload::Response )
    {
      request_string  = payload->_version;
      request_string += ' ';
      request_string += getStringFromResponse( payload->_response );
      request_string += (char) 13;
      request_string += (char) 10;
    }
    else // Request
    {
      request_string  = MethodStrings[ payload->_method ];
      request_string += ' ';
      request_string += payload->_request;
      request_string += ' ';
      request_string += payload->_version;
      request_string += (char) 13;
      request_string += (char) 10;
    }

    for ( HTTPPayload::HeaderData::const_iterator it = payload->_header.begin(); it != payload->_header.end(); ++it )
    {
      header_string += it->first;
      header_string += ": ";
      header_string += it->second;
      header_string += (char) 13;
      header_string += (char) 10;
    }

    size_t buffer_size = request_string.size() + header_string.size() + payload->_body.size();

    Buffer* buffer = new Buffer( buffer_size );

    buffer->push( request_string );

    buffer->push( header_string );

    buffer->push( (char) 13 ); // CR
    buffer->push( (char) 10 ); // LF

    if ( payload->_isFile )
    {
      std::ifstream instream( payload->_body, std::ios_base::in );
      buffer->push( instream );
    }
    else
    {
      buffer->push( payload->_body );
    }

    buffer->push( (char) 13 ); // CR
    buffer->push( (char) 10 ); // LF

    this->pushBuffer( buffer );
  }


  void HTTPSerializer::deserialize( const Buffer* buffer )
  {
    DEBUG_LOG( "Stewardess::HTTPSerializer", "Deserializing" );

    Buffer::Iterator current = buffer->getIterator();

    HTTPPayload* payload = new HTTPPayload();

    std::stringstream request_string;
    std::string key_string;
    std::string value_string;

//////////////////// // Load the Request
    while ( current )
    {
      if ( *current == (char)13 )
      {
        ++current;
        if ( *current == (char)10 )
        {
          ++current;
          break;
        }
        else
        {
          delete payload;
          payload = nullptr;
          this->pushError( ErrorMalformedPayload );
          return;
        }
      }
      else
      {
        request_string << *current;
      }
      ++current;
    }

    std::string request_token;
    request_string >> request_token;

    payload->_method = getMethodFromName( request_token );
    if ( payload->_method == HTTPPayload::Response )
    {
      // request_token contains the version
      payload->_version = request_token;

      int response_code;
      request_string >> response_code;
      payload->_response = (HTTPPayload::ResponseType)response_code;
    }
    else
    {
      request_string >> payload->_request;
      request_string >> payload->_version;
    }


//////////////////// // Load the Header
    while ( current )
    {
      bool key = true;
      while ( current )
      {
        if ( *current == (char)13 )
        {
          ++current;
          if ( current && *current == (char)10 )
          {
            payload->addHeader( key_string, value_string );
            key_string.clear();
            value_string.clear();
            ++current;
            break;
          }
          else
          {
            delete payload;
            payload = nullptr;
            this->pushError( ErrorMalformedPayload );
            return;
          }
        }
        else
        {
          if ( key )
          {
            if ( *current == ':' )
            {
              ++current; // Expect a space
              if ( ! current )
              {
                delete payload;
                payload = nullptr;
                this->pushError( ErrorMalformedPayload );
                return;
              }
              key = false;
            }
            else
            {
              key_string.push_back( *current );
            }
          }
          else
            value_string.push_back( *current );
        }
        ++current;
      }

      if ( *current == (char)13 )
      {
        ++current;
        if ( current && *current == (char)10 )
        {
          ++current;
          break;
        }
        else
        {
          delete payload;
          payload = nullptr;
          this->pushError( ErrorMalformedPayload );
          return;
        }
      }
    }

//////////////////// // Load the Body
    while ( current )
    {
      payload->_body.push_back( *current );
      ++current;
    }

    this->pushPayload( payload );
  }


////////////////////////////////////////////////////////////////////////////////////////////////////

  HTTPPayload::HTTPPayload() :
    _method( Response ),
    _request(),
    _version(),
    _response( Null ),
    _header(),
    _isFile( false ),
    _body()
  {
  }


  HTTPPayload::HTTPPayload( ResponseType response, std::string bodyText ) :
    _method( Response ),
    _request(),
    _version( VersionString ),
    _response( response ),
    _header(),
    _isFile( false ),
    _body( bodyText )
  {
  }


  void HTTPPayload::setHeader( std::string key, std::string value )
  {
    _header[key] = value;
  }


  void HTTPPayload::addHeader( std::string key, std::string value )
  {
    HeaderData::iterator found = _header.find( key );
    if ( found != _header.end() )
    {
      found->second += ", ";
      found->second += value;
    }
    else
    {
      _header[key] = value;
    }
  }


  std::string HTTPPayload::getHeader( std::string key ) const
  {
    HeaderData::const_iterator found = _header.find( key );
    if ( found == _header.end() )
    {
      return std::string("");
    }
    else
    {
      return found->second;
    }
  }

  std::string HTTPPayload::print() const
  {
    std::stringstream ss;
    if ( _method == Response )
    {
      ss << _version << ' ' << getStringFromResponse( _response ) << '\n';
    }
    else
    {
      ss << MethodStrings[ _method ] << ' ' << _request << ' ' << VersionString << '\n';
    }
    for ( HeaderData::const_iterator it = _header.begin(); it != _header.end(); ++it )
    {
      ss << it->first << ": " << it->second << '\n';
    }
    ss << '\n';
    ss << _body;

    return ss.str();
  }


////////////////////////////////////////////////////////////////////////////////////////////////////

  HTTPPayload::MethodType getMethodFromName( std::string name  )
  {
    for ( size_t i = 0; i < 10; ++i )
    {
      if ( name == MethodStrings[i] )
        return (HTTPPayload::MethodType)i;
    }

    return HTTPPayload::Response;
  }


  std::string getStringFromResponse( HTTPPayload::ResponseType response )
  {
    switch( response )
    {
      case HTTPPayload::Ok :
        return std::string( "200 OK" );
        break;

      case HTTPPayload::Created :
        return std::string( "201 Created" );
        break;

      case HTTPPayload::Accepted :
        return std::string( "202 Accpeted" );
        break;

      case HTTPPayload::BadRequest :
        return std::string( "400 Bad Request" );
        break;

      case HTTPPayload::Unauthorized :
        return std::string( "401 Unauthorized" );
        break;

      case HTTPPayload::PaymentRequired :
        return std::string( "402 Payment Required" );
        break;

      case HTTPPayload::Forbidden :
        return std::string( "403 Forbidden" );
        break;

      case HTTPPayload::NotFound :
        return std::string( "404 Not Found" );
        break;

      case HTTPPayload::MethodNotAllowed :
        return std::string( "405 Method Not Allowed" );
        break;

      case HTTPPayload::InternalServerError :
        return std::string( "500 Internal Server Error" );
        break;

      case HTTPPayload::NotImplemented :
        return std::string( "501 Not Implemented" );
        break;

      case HTTPPayload::BadGateway :
        return std::string( "502 Bad Gateway" );
        break;

      case HTTPPayload::ServiceUnavailable :
        return std::string( "503 Service Unavailable" );
        break;

      case HTTPPayload::GatewayTimeout :
        return std::string( "504 Gateway Timeout" );
        break;

      case HTTPPayload::HTTPVersionNotSupported :
        return std::string( "505 HTTP Version Not Supported" );
        break;

      default :
        return std::string( "500 Internal Server Error" );
        break;
    }
//    return std::string( "200 OK" );
  }

}

