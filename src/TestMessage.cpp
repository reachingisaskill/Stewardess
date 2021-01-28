
#include "TestMessage.h"


TestMessageBuilder::TestMessageBuilder() :
  _currentString(),
  _isBuilding( false ),
  _isBuilt( false )
{
  _currentString.reserve( 4096 );
}


TestMessageBuilder::~TestMessageBuilder()
{
}


MessageBase* TestMessageBuilder::getMessage()
{
  _isBuilt = false;
  return new TestMessage( _currentString );
}


void TestMessageBuilder::build( char c )
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

