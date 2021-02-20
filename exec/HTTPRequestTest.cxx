
#include "HTTPSerializer.h"

#include <iostream>
#include <cstring>
#include <cassert>


using namespace Stewardess;


int main( int, char** )
{

  HTTPRequest request1( "/some/path/to/a/file.html" );

  assert( request1.getPath() == "/some/path/to/a/file.html" );
  assert( request1.size() == 0 );

  std::cout << "Passed: " << request1.encode() << std::endl;


  HTTPRequest request2( "/some/path/to/a/file.html?key1=value1&key2=value2#bottom" );

  assert( request2.getPath() == "/some/path/to/a/file.html" );
  assert( request2.size() == 2 );
  assert( request2.getFragment() == "bottom" );
  assert( request2.getQuery("key1") == std::string( "value1" ) );
  assert( request2.getQuery("key2") == std::string( "value2" ) );

  std::cout << "Passed: " << request2.encode() << std::endl;


  HTTPRequest request3( "/blah/hello/there/a_file.txt?this%20is%20a%20string=some%2Dvalue" );

  assert( request3.getPath() == "/blah/hello/there/a_file.txt" );
  assert( request3.size() == 1 );
  assert( request3.getFragment() == "" );
  assert( request3.getQuery("this is a string") == std::string( "some-value" ) );

  std::cout << "Passed: " << request3.encode() << std::endl;

  return 0;
}
