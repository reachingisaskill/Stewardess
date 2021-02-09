
#ifndef STEWARDESS_BUFFER_H_
#define STEWARDESS_BUFFER_H_

#include "Definitions.h"

#include <utility>
#include <string>
#include <cstring>


namespace Stewardess
{
  class Buffer
  {
    private:
      // Internal chunk data type
      struct Chunk
      {
        size_t capacity;
        size_t size;

        Chunk* next;

        char* data;

        // Construct empty
        explicit Chunk( size_t );
        // Aquire character array
        explicit Chunk( char*, size_t );
        // Delete memory
        ~Chunk();

        // Reallocate the capacity of this chunk
        void reallocate( size_t );


        Chunk( Chunk&& ) = default;

        Chunk( const Chunk& ) = delete;
        Chunk& operator=( const Chunk& ) = delete;
        Chunk& operator=( const Chunk&& ) = delete;
      };


    public:
      class Iterator
      {
        friend class Buffer;
        private:
          const Chunk* _chunk;
          size_t _position;

          // Private constructor. Only the buffer can construct its iterators
          Iterator( const Chunk* );
        public:

          // Increment the position
          void increment();

          // Return the character at the current position
          char operator*() const;

          // Increment the iterators position
          void operator++() { this->increment(); }

          // Return true while the iterator is valid
          operator bool() const;
      };

    private:
      // The size we make the chunks
      size_t _maxChunkSize;

      // First chunk
      Chunk* _start;

      // Last chunk
      Chunk* _finish;

      // Allocate a new chunk of the requested size and append it
      void allocate();

    public:

      // Construct a buffer specifying the chunk size
      explicit Buffer( size_t = 1000 );

      // Copy the underlying data
      Buffer( const Buffer& );
      Buffer& operator=( const Buffer& );

      // Move the data pointers
      Buffer( Buffer&& ) = default;
      Buffer& operator=( Buffer&& );

      // Destructor
      ~Buffer();


      // Sum the capacity of all the chunks
      size_t getCapacity() const;

      // Sum the number of chars written to all the chunks
      size_t getSize() const;

      // Sum the number of chunks
      size_t getNumberChunks() const;

      // The suggested size for data chunks
      size_t allocationSize() const { return _maxChunkSize; }



      // Deletes everything
      void clear();



      // Interface for users to push strings to the buffer
      // Copies data into the last chunk, allocating a new one as required
      void push( std::string& );
      void push( char );



      // Interface for users to inspect a buffer
      Iterator getIterator() const;



      // Interface for reading from sockets!
      // Adds a chunk based on the allocated character array
      void pushChunk( char*, size_t );



      // Interface for writing to sockets!
      // Return a read iterator pointing to the first chunk
      const char* chunk() const;
      size_t chunkSize() const;
      // Removes the first chunk
      void popChunk();



      // Returns true if the buffer is empty
      bool empty() const { return _start == nullptr; }

      // Return the logical complement of empty
      operator bool() const { return _start != nullptr; }


      // Useful for debugging
      std::string getString() const;

  };

}

#endif // STEWARDESS_BUFFER_H_

