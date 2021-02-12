
#include "Buffer.h"

#include <cstring>


namespace Stewardess
{

////////////////////////////////////////////////////////////////////////////////////////////////////
  // Chunk member function definitions

  Buffer::Chunk::Chunk( size_t c ) :
    capacity( c ),
    size( 0 ),
    next( nullptr ),
    data( new char[ c ] )
  {
  }


  Buffer::Chunk::Chunk( char* data, size_t size ) :
    capacity( size ),
    size( size ),
    next( nullptr ),
    data( data )
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
      capacity = cap;
      std::memcpy( data, old_data, size );
    }
    else
    {
      size = cap;
      capacity = cap;
      std::memcpy( data, old_data, size );
    }

    delete[] old_data;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Iterator member function definitions


  Buffer::Iterator::Iterator( const Chunk* chunk ) :
    _chunk( chunk ),
    _position( 0 )
  {
  }


  char Buffer::Iterator::operator*() const
  {
    return _chunk->data[ _position ];
  }


  void Buffer::Iterator::increment()
  {
    if ( ++_position >= _chunk->size )
    {
      _position = 0;
      _chunk = _chunk->next;
    }
  }


  Buffer::Iterator::operator bool() const
  {
    return _chunk != nullptr;
  }


////////////////////////////////////////////////////////////////////////////////////////////////////
  // Buffer member function definitions

  Buffer::Buffer( size_t c ) :
    _maxChunkSize( c ),
    _start( nullptr ),
    _finish( nullptr )
  {
  }


  Buffer::~Buffer()
  {
    this->clear();
  }


  Buffer::Buffer( const Buffer& other ) :
    _maxChunkSize( other._maxChunkSize ),
    _start( nullptr ),
    _finish( nullptr )
  {
    Chunk* current = other._start;

    while ( current != nullptr )
    {
      if ( _start == nullptr )
      {
        _finish = new Chunk( current->capacity );
        _start = _finish;
        std::memcpy( _finish->data, current->data, current->size );
      }
      else
      {
        _finish->next = new Chunk( current->capacity );
        _finish = _finish->next;
        std::memcpy( _finish->data, current->data, current->size );
      }
      current = current->next;
    }
  }


  Buffer& Buffer::operator=( const Buffer& other )
  {
    this->clear();
    _start = nullptr;
    _finish = nullptr;

    _maxChunkSize = other._maxChunkSize;

    Chunk* current = other._start;

    while ( current != nullptr )
    {
      if ( _start == nullptr )
      {
        _finish = new Chunk( current->capacity );
        _start = _finish;
        std::memcpy( _finish->data, current->data, current->size );
      }
      else
      {
        _finish->next = new Chunk( current->capacity );
        _finish = _finish->next;
        std::memcpy( _finish->data, current->data, current->size );
      }
      current = current->next;
    }

    return *this;
  }


  Buffer& Buffer::operator=( Buffer&& other )
  {
    this->clear();

    _maxChunkSize = std::move( other._maxChunkSize );
    _start = std::exchange( other._start, nullptr );
    _finish = std::exchange( other._finish, nullptr );

    return *this;
  }


  void Buffer::allocate()
  {
    if ( _start != nullptr )
    {
      _finish->next = new Chunk( _maxChunkSize );
      _finish = _finish->next;
    }
    else
    {
      _finish = new Chunk( _maxChunkSize );
      _start = _finish;
    }
  }


  size_t Buffer::getCapacity() const
  {
    Chunk* current = _start;
    size_t counter = 0;
    while( current != nullptr )
    {
      counter += current->capacity;
      current = current->next;
    }
    return counter;
  }


  size_t Buffer::getSize() const
  {
    Chunk* current = _start;
    size_t counter = 0;
    while( current != nullptr )
    {
      counter += current->size;
      current = current->next;
    }
    return counter;
  }


  size_t Buffer::getNumberChunks() const
  {
    Chunk* current = _start;
    size_t counter = 0;
    while( current != nullptr )
    {
      counter += 1;
      current = current->next;
    }
    return counter;
  }


  void Buffer::clear()
  {
    while ( _start != nullptr )
    {
      Chunk* temp = _start;
      _start = _start->next;
      delete temp;
    }
  }


  void Buffer::push( std::istream& stream )
  {
    if ( _start == nullptr )
      this->allocate();

    if ( stream )
    {
      stream.read( &_finish->data[_finish->size], _finish->capacity - _finish->size );
      _finish->size += stream.gcount();

      while ( stream )
      {
        this->allocate();
        stream.read( &_finish->data[_finish->size], _finish->capacity - _finish->size );
        _finish->size += stream.gcount();
      }
    }
  }


  void Buffer::push( std::string& string )
  {
    if ( _start == nullptr )
      this->allocate();

    size_t string_start = 0;
    size_t remaining = _finish->capacity - _finish->size;
    size_t string_remaining = string.size() - string_start;

    while ( string_remaining > remaining )
    {
      std::memcpy( &_finish->data[_finish->size], &string.c_str()[ string_start ], remaining );
      _finish->size += remaining;
      string_start += remaining;
      string_remaining -= remaining;
      this->allocate();
      remaining = _finish->capacity - _finish->size;
    }

    std::memcpy( &_finish->data[_finish->size], &string.c_str()[string_start], string_remaining );
    _finish->size += string_remaining;
  }


  void Buffer::push( char c )
  {
    if ( _start == nullptr )
      this->allocate();

    if ( _finish->capacity == _finish->size )
    {
      this->allocate();
    }

    _finish->data[_finish->size] = c;
    _finish->size += 1;
  }


  void Buffer::pushChunk( char* data, size_t size )
  {
    if ( _start != nullptr )
    {
      _finish->next = new Chunk( data, size );
      _finish = _finish->next;
    }
    else
    {
      _finish = new Chunk( data, size );
      _start = _finish;
    }
  }


  Buffer::Iterator Buffer::getIterator() const
  {
    return Iterator( _start );
  }


  const char* Buffer::chunk() const
  {
    return _start->data;
  }


  size_t Buffer::chunkSize() const
  {
    return _start->size;
  }


  void Buffer::popChunk()
  {
    if ( _start != nullptr )
    {
      Chunk* temp = _start;
      _start = _start->next;
      delete temp;
    }
  }


  std::string Buffer::getString() const
  {
    std::string result;

    Chunk* current = _start;

    while ( current != nullptr )
    {
      for ( size_t i = 0; i < current->size; ++i )
      {
        result.push_back( current->data[i] );
      }
      current = current->next;
    }

    return result;
  }

}

