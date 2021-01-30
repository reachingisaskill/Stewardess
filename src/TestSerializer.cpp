
#include "TestSerializer.h"
#include "Buffer.h"

#include <cstring>


static const char* ErrorIncompletePayload = "New payload started before previous has finished.";
static const char* ErrorUnexpectedData = "Unexpected data found between payloads.";


TestSerializer::TestSerializer() :
  _currentPayload()
{
}


TestSerializer::~TestSerializer()
{
}


void TestSerializer::serialize( const Payload* p )
{
  const std::string& message = ((TestPayload*)p)->getMessage();
  Buffer* buffer = new Buffer( message.size() );

  for ( size_t i = 0; i < message.size(); ++i )
  {
    buffer->push( message[i] );
  }

  buffer->resize( message.size() );
  std::cout << "Writing response" << std::endl;

  this->pushBuffer( buffer );
}


void TestSerializer::deserialize( const Buffer* buffer )
{
  // Iterate through and break into messages
  for ( Buffer::const_iterator it = buffer->begin(); it != buffer->end(); ++it )
  {
    if ( _currentPayload.empty() )
    {
      if ( (*it) == '{' ) // Wait for the start of the message. Otherwise it is classed as garbage.
      {
        _currentPayload.clear();
        _currentPayload.push_back( (*it) );
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
        _currentPayload.push_back( (*it) );
        continue;
      }

      _currentPayload.push_back( (*it) );

      if ( (*it) == '}' )
      {
        this->pushPayload( new TestPayload( _currentPayload ) );
        _currentPayload.clear();
      }
    }
  }
}

