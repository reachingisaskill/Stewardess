
#include "Exception.h"


namespace Stewardess
{

  Exception::Exception( const char* data ) :
    std::runtime_error( "Stewardess exception" ),
    _what( data )
  {
  }


  Exception::Exception( const std::string& data ) :
    std::runtime_error( "Stewardess exception" ),
    _what( data )
  {
  }


  const char* Exception::what() const noexcept
  {
    return _what.c_str();
  }

}

