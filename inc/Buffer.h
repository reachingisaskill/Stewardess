
#ifndef STEWARDESS_BUFFER_H_
#define STEWARDESS_BUFFER_H_

#include "Definitions.h"


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

  };

}

#endif // STEWARDESS_BUFFER_H_

