
#include "Buffer.h"


namespace Stewardess
{

  Buffer::Buffer() :
    _capacity( 1 ),
    _data( new char[1] ),
    _size( 0 )
  {
  }


  Buffer::Buffer( size_t c ) :
    _capacity( c ),
    _data( new char[ _capacity ] ),
    _size( 0 )
  {
  }


  Buffer::~Buffer()
  {
    delete[] _data;
    _data = nullptr;
  }


  Buffer::Buffer( const Buffer& other ) :
    _capacity( other._capacity ),
    _data( new char[ _capacity ] ),
    _size( other._size )
  {
    for ( size_t i = 0; i < _size; ++i )
    {
      _data[i] = other._data[i];
    }
  }


  Buffer& Buffer::operator=( const Buffer& other )
  {
    delete[] _data;

    _capacity = other._capacity;
    _data = new char[ _capacity ];
    _size = other._size;

    for ( size_t i = 0; i < _size; ++i )
    {
      _data[i] = other._data[i];
    }

    return *this;
  }


  Buffer& Buffer::operator=( Buffer&& other )
  {
    delete[] _data;

    _capacity = std::exchange( other._capacity, 1 );
    _data = std::exchange( other._data, new char );
    _size = std::exchange( other._size, 0 );

    return *this;
  }


  void Buffer::push( char c )
  {
    _data[_size] = c;
    ++_size;
  }


  void Buffer::reserve( size_t new_cap )
  {
    delete[] _data;

    _capacity = new_cap;
    _data = new char[ _capacity ];

    if ( _size > _capacity )
    {
      _size = _capacity;
    }
  }


  void Buffer::resize( size_t new_size )
  {
    _size = new_size;
  }

}

