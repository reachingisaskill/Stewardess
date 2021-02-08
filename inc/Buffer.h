
#ifndef STEWARDESS_BUFFER_H_
#define STEWARDESS_BUFFER_H_

#include "Definitions.h"

#include <cstring>


namespace Stewardess
{
  class Buffer
  {
    public :
      // Iterator types are trivial
      typedef char* iterator;
      typedef const char* const_iterator;

    private:
      // To total accesible length
      size_t _capacity;
      // The raw data
      char* _data;
      // The length of the raw data
      size_t _size;

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

      // Move the data pointer
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

