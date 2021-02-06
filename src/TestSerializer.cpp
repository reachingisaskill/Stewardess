
#include "TestSerializer.h"
#include "Buffer.h"

#include <cstring>


namespace Stewardess
{

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
    std::cout << "Serializing" << std::endl;
    const std::string& message = ((TestPayload*)p)->getMessage();
    Buffer* buffer = new Buffer( message.size()+2 );
    buffer->push( '{' );

    for ( size_t i = 0; i < message.size(); ++i )
    {
      buffer->push( message[i] );
    }

    buffer->push( '}' );

    buffer->resize( message.size()+2 );

    this->pushBuffer( buffer );
  }


  void TestSerializer::deserialize( const Buffer* buffer )
  {
    std::cout << "Deserializing" << std::endl;
    bool building = false;
    // Iterate through and break into messages
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

}

