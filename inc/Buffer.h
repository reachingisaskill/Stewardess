
#ifndef STEWARDESS_BUFFER_H_
#define STEWARDESS_BUFFER_H_

#include "Definitions.h"

#include <cstring>


namespace Stewardess
{
  class Buffer
  {
    public:
    const size_t MaxChunkSize = 4096;

    struct Chunk
    {
      size_t capacity;
      size_t size;

      Chunk* next;

      char* data;

      Chunk( size_t, Chunk*, Chunk* );
      ~Chunk();

      void reallocate( size_t );

      Chunk( const Chunk& ) = delete;
      Chunk( const Chunk&& ) = delete;
      Chunk& operator=( const Chunk& ) = delete;
      Chunk& operator=( const Chunk&& ) = delete;
    };

    public :
      // Iterator types are trivial
      typedef char* iterator;
      typedef const char* const_iterator;

    private:
      // To total accesible length
      size_t _capacity;
      // The used length of the raw data
      size_t _size;

      // First chunk
      Chunk* _start;

      // Last chunk
      Chunk* _finish;

      // Allocate the capacity
      void allocate( size_t );

      // Reallocate, copying the data
      void reallocate( size_t );

      // Clear all the chunks
      void deallocate( Chunk* = nullptr );

    public:

      // Default construction
      Buffer();

      // Construct with a capacity
      explicit Buffer( size_t );

      // Destructor
      ~Buffer();


      // Copy the underlying data
      Buffer( const Buffer& );
      Buffer& operator=( const Buffer& );

      // Move the data pointers
      Buffer( Buffer&& ) = default;
      Buffer& operator=( Buffer&& );


      // Return the underlying data
      const char* data() const { return _data; }
      char* data() { return _data; }


      // Push a character to the end
      void push( char );

      // Push a string character by character to the end
      void push( const std::string& );


      // Bytewise copy into the buffer
      template < typename DATA >
      void bytePush( DATA& );

      // Bytewise copy out of the buffer
      template < typename DATA >
      void bytePop( size_t, DATA& ) const;


      // Return the character at position n
      char at( size_t n ) const { return _data[n]; }


      // Change the capacity of the buffer
      void reserve( size_t );

      // Return the accesible capacity
      size_t capacity() const { return _capacity; }

      // Set the size following a write
      void resize( size_t );

      // Return the last size set
      size_t size() const { return _size; }


      // Return basic iterators
      constexpr iterator begin() { return _data; }
      constexpr const_iterator begin() const { return _data; }
      constexpr iterator end() { return &_data[_size]; }
      constexpr const_iterator end() const { return &_data[_size]; }


      // Useful for debugging
      std::string getString() const;

  };


  template < typename DATA >
  void Buffer::bytePush( DATA& data )
  {
    std::memcpy( (void*)&_data[_size], (void*)&data, sizeof( DATA ) );
    _size += sizeof( DATA );
  }


  template < typename DATA >
  void Buffer::bytePop( size_t pos, DATA& data ) const
  {
    std::memcpy( (void*)&data, (void*)&_data[pos], sizeof( DATA ) );
  }

}

#endif // STEWARDESS_BUFFER_H_

