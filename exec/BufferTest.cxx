
#include "Buffer.h"

using namespace Stewardess;


Buffer buildFunc();


int main( int, char** )
{
  {
    Buffer b( 1000 );
    std::string dots( "........" );

    std::cout << "Expect Capacity 0 : " << b.getCapacity() << std::endl;
    std::cout << "Expect chunks 0 : " << b.getNumberChunks() << std::endl;
    std::cout << "Expect size 0 : " << b.getSize() << std::endl;
    std::cout << "Expect 0 : " << b << std::endl;

    char* temp = new char[ 999 ];
    b.pushChunk( temp, 999 );

    std::cout << "Expect Capacity 999 : " << b.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b.getNumberChunks() << std::endl;
    std::cout << "Expect size 999 : " << b.getSize() << std::endl;
    std::cout << "Expect 1 : " << b << std::endl;

    Buffer b1( 10 );
    b1.push( dots );

    std::cout << "Expect Capacity 10 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 8 : " << b1.getSize() << std::endl;
    std::cout << "Expect 1 : " << b << std::endl;

    b1.push( dots );

    std::cout << "Expect Capacity 20 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 2 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 16 : " << b1.getSize() << std::endl;
    std::cout << "Expect 1 : " << b << std::endl;
  }

  std::cout << "\n----------------------------------------------------------------------------------------------------\n" << std::endl;

  {
    Buffer b1 = buildFunc();

    std::cout << "Expect Capacity 175 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 3 : " << b1.getNumberChunks() << std::endl;

    b1.popChunk();

    std::cout << "Expect Capacity 75 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 2 : " << b1.getNumberChunks() << std::endl;

    Buffer b2( 1000 );

    std::cout << "Expect Capacity 1000 : " << b2.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b2.getNumberChunks() << std::endl;

    b2 = buildFunc();

    std::cout << "Expect Capacity 175 : " << b2.getCapacity() << std::endl;
    std::cout << "Expect chunks 3 : " << b2.getNumberChunks() << std::endl;

    b2 = Buffer( 432 );

    std::cout << "Expect Capacity 432 : " << b2.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b2.getNumberChunks() << std::endl;

    Buffer b3( b2 );

    std::cout << "Expect Capacity 432 : " << b3.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b3.getNumberChunks() << std::endl;
  }

  std::cout << "\n----------------------------------------------------------------------------------------------------\n" << std::endl;

  {
    std::string string3 = "...";
    std::string string6 = "......";
    std::string string20 = "....................";
    Buffer b1( 5 );

    std::cout << "Expect Capacity 0 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 0 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 0 : " << b1.getSize() << std::endl;

    b1.push( string3 );

    std::cout << "Expect Capacity 5 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 1 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 3 : " << b1.getSize() << std::endl;

    b1.push( string3 );

    std::cout << "Expect Capacity 10 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 2 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 6 : " << b1.getSize() << std::endl;

    b1.push( string6 );

    std::cout << "Expect Capacity 15 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 3 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 12 : " << b1.getSize() << std::endl;

    b1.popChunk();

    std::cout << "Expect Capacity 10 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 2 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 7 : " << b1.getSize() << std::endl;

    b1.push( 'a' );

    std::cout << "Expect Capacity 10 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 2 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 8 : " << b1.getSize() << std::endl;

    b1.push( string20 );

    std::cout << "Expect Capacity 30 : " << b1.getCapacity() << std::endl;
    std::cout << "Expect chunks 6 : " << b1.getNumberChunks() << std::endl;
    std::cout << "Expect size 28 : " << b1.getSize() << std::endl;

    std::cout << "String   = " << b1.getString() << std::endl;
    std::cout << "Iterator = ";

    for( Buffer::Iterator it = b1.getIterator(); it; ++it )
    {
      std::cout << *it;
    }
    std::cout << std::endl;
  }

  std::cout << "\n----------------------------------------------------------------------------------------------------\n" << std::endl;

  {
    Buffer b( 100 );
    std::string filename( "./README.md" );

    std::cout << "Expect Capacity 0 : " << b.getCapacity() << std::endl;
    std::cout << "Expect chunks 0 : " << b.getNumberChunks() << std::endl;
    std::cout << "Expect size 0 : " << b.getSize() << std::endl;
    std::cout << "Expect 0 : " << b << std::endl;

    std::ifstream instream( filename, std::ios_base::in );

    b.push( instream );

    std::cout << "Expect Capacity 800 : " << b.getCapacity() << std::endl;
    std::cout << "Expect chunks 8 : " << b.getNumberChunks() << std::endl;
    std::cout << "Expect size 798 : " << b.getSize() << std::endl;
    std::cout << "Expect 1 : " << b << std::endl;
  }


  return 0;
}


Buffer buildFunc()
{
  Buffer b( 100 );

  char* temp = new char[ 100 ];
  b.pushChunk( temp, 100 );
  temp = new char[ 50 ];
  b.pushChunk( temp, 50 );
  temp = new char[ 25 ];
  b.pushChunk( temp, 25 );

  return b;
}

