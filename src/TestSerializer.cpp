
#include "TestSerializer.h"

#include <cstring>


TestSerializer::TestSerializer() :
  _currentString(),
  _buffer(),
  _isBuilding( false ),
  _isBuilt( false )
{
  _currentString.reserve( 4096 );
}


TestSerializer::~TestSerializer()
{
}


void TestSerializer::serialize( Payload* p )
{
  _buffer.clear();
  std::string& message = ((TestPayload*)p)->getMessage();
  std::copy( message.begin(), message.end(), std::back_inserter(_buffer) );
}


const char* TestSerializer::payloadBuffer()
{
  return &_buffer[0];
}


size_t TestSerializer::payloadBufferSize()
{
  return _buffer.size();
}


Payload* TestSerializer::getPayload()
{
  _isBuilt = false;
  return new TestPayload( _currentString );
}


void TestSerializer::build( char c )
{
  if ( _isBuilding )
  {
    if ( c == '{' )
    {
      // ERROR...
    }

    _currentString.push_back( c );

    if ( c == '}' )
    {
      _isBuilt = true;
      _isBuilding = false;
    }
  }
  else
  {
    if ( c == '{' )
    {
      _currentString.clear();
      _currentString.push_back( c );
      _isBuilding = true;
    }
  }
}

