
#include "Buffer.h"

#include <cstring>


namespace Stewardess
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Chunk member function definitions

  Buffer::Chunk::Chunk( size_t c, Chunk* next ) :
    capacity( c ),
    size( 0 ),
    next( next ),
    data( new char[ c ] )
  {
  }


  Buffer::Chunk::~Chunk()
  {
    delete[] data;
  }


  void Buffer::Chunk::reallocate( size_t cap )
  {
    char* old_data = data;

    data = new char[ cap ];

    if ( cap >= size )
    {
      capacity = cap
      std::memcpy( data, old_data, size );
    }
    else
    {
      size = cap;
      capacity = cap
      std::memcpy( data, old_data, size );
    }

    delete[] old_data;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Buffer member function definitions

  Buffer::Buffer() :
    _capacity( 0 ),
    _size( 0 ),
    _start( nullptr ),
    _finish( nullptr )
  {
  }


  Buffer::Buffer( size_t c ) :
    _capacity( 0 ),
    _size( 0 ),
    _start( nullptr ),
    _finish( nullptr )
  {
    this->allocate( c );
  }


  Buffer::~Buffer()
  {
    this->deallocate();
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


  void Buffer::allocate( size_t capacity )
  {
    while( capacity > MaxChunkSize )
    {
      if ( _finish == nullptr )
      {
        _finish = new Chunk( MaxChunkSize, nullptr );
        _start = _finish;
      }
      else
      {
        _finish->next = new Chunk( MaxChunkSize, nullptr );
        _finish = finish->next;
      }
      capacity -= MaxChunkSize;
    }

    _finish->next = new Chunk( capacity, nullptr );
    _finish = finish->next;
  }


  void Buffer::reallocate( size_t capacity )
  {
    size_t current_old_capacity = 0;
    size_t remaining_capacity = capacity;

    Chunk* current = _start;

    while ( current != nullptr )
    {
      current_old_capacity += current->capacity;

      if ( current_old_capacity > capacity ) // Resize down and delete from here onwards
      {
        current->reallocate( remaining_capacity );
        this->deallocate( current );
        break;
      }

      remaining_capacity -= current->capacity;
      current = current->next;
    }

    if ( capacity > current_old_capacity ) // Resize up and add new chunks
    {
      this->allocate( remaining_capacity );
    }
  }


  void Buffer::deallocate( Chunk* from )
  {
    Chunk* current;

    if ( from == nullptr )
    {
      current = _start;
      _start = nullptr;
      _finish = nullptr;
    }
    else
    {
      current = from->next;
      _finish = from;
    }

    while ( current != nullptr )
    {
      Chunk* temp = current;
      current = current->next;
      delete temp;
    }
  }


  void Buffer::push( char c )
  {
    _data[_size] = c;
    ++_size;
  }


  void Buffer::push( const std::string& string )
  {
    std::memcpy( &_data[_size], string.c_str(), string.size() );
    _size += string.size();
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


  std::string Buffer::getString() const
  {
    std::string result;
    for ( size_t i = 0; i < _size; ++i )
    {
      char c = _data[i];
      if ( (int)c < 32 )
      {
        result += std::string( "(&" ) + std::to_string( (int)c ) + std::string( ")" );
      }
      else
        result.push_back( _data[i] );
    }
    return result;
  }

}

