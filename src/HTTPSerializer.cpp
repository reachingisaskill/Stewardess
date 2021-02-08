
#include "HTTPSerializer.h"
#include "Buffer.h"

#include <cstring>


namespace Stewardess
{

  HTTPPayload::HeaderField makeHeaderField( const char*, const char* );

  const char* const MethodStrings[] = { "RESPONSE", "GET", "POST" };
  const size_t MethodStringLengths[] = { 8, 3, 4 };

  const char* const ResponseStrings[] = { "200 OK", "404 NotFound" };
  const size_t ResponseStringLengths[] = { 6, 12 };

  const char* const VersionString = "HTTP/1.1";
  const size_t VersionStringLength = 8;


////////////////////////////////////////////////////////////////////////////////////////////////////


  static const char* ErrorIncompletePayload = "New payload started before previous has finished.";
  static const char* ErrorUnexpectedData = "Unexpected data found between payloads.";


////////////////////////////////////////////////////////////////////////////////////////////////////


  void HTTPSerializer::serialize( const Payload* p )
  {
    DEBUG_LOG( "Stewardess::HTTPSerializer", "Serializing" );

    HTTPPayload payload = (HTTPPayload*)p;
    std::string request_string;
    std::string header_string;

    if ( payload->_method == HTTPPayload::Response )
    {
      request_string  = VersionString;
      request_string += ' ';
      request_string += ResponseStrings[ payload->_response ];
      request_string += (char) 13;
      request_string += (char) 10;
    }
    else // Request
    {
      request_string  = MethodStrings[ payload->_method ];
      request_string += ' ';
      request_string += payload->_request;
      request_string += ' ';
      request_string += VersionString;
      request_string += (char) 13;
      request_string += (char) 10;
    }

    for ( size_t i = 0; i < p->_header.size(); ++i )
    {
      header_string += payload->_header[i].first;
      header_string += ": "
      header_string += payload->_header[i].second;
    }
    header_string += (char) 13;
    header_string += (char) 10;


    size_t buffer_size = request_string.size() + header_string.size() + payload->_body.size();

    Buffer* buffer = new Buffer( buffer_size );

    buffer.push( request_string );

    buffer.push( header_string );

    buffer.push( (char) 13 ); // CR
    buffer.push( (char) 10 ); // LF

    buffer.push( payload->_body );

    this->pushBuffer( buffer );
  }


  void HTTPSerializer::deserialize( const Buffer* buffer )
  {
    DEBUG_LOG( "Stewardess::HTTPSerializer", "Deserializing" );

    for ( Buffer::const_iterator it = buffer->begin(); it != buffer->end(); ++it )
    {
      if ( ! building )
      {
        if ( (*it) == '{' ) // Wait for the start of the message. Otherwise it is classed as garbage.
        {
          _currentPayload.clear();
          building = true;
        }
        else
        {
          this->pushError( ErrorUnexpectedData );
          std::cout << " THE ERROR = " << int( (*it ) );
        }
      }
      else
      {
        if ( (*it) == '{' )
        {
          this->pushError( ErrorIncompletePayload );
          _currentPayload.clear();
        }
        else if ( (*it) == '}' )
        {
          this->pushPayload( new TestPayload( _currentPayload ) );
          _currentPayload.clear();
          building = false;
        }
        else
        {
          _currentPayload.push_back( (*it) );
        }
      }
    }
  }


////////////////////////////////////////////////////////////////////////////////////////////////////

  HTTPPayload::HTTPPayload() :
    _method( Response ),
    _request(),
    _response(),
    _header(),
    _body()
  {
  }


  HTTPPayload::HTTPPayload( std::string response, std::string bodyText ) :
    _method( Response ),
    _request(),
    _response( response ),
    _header(),
    _body( bodyText )
  {

  }


  HTTPPayload::HeaderField makeHeaderField( const char* key, const char* value )
  {
    return std::make_pair( std::string( key ), std::string( value ) );
  }

}

