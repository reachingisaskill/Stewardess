
#ifndef STEWARDESS_REFERENCE_COUNTER_H_
#define STEWARDESS_REFERENCE_COUNTER_H_

#include "Definitions.h"
#include <mutex>
#include <atomic>


namespace Stewardess
{

  struct ReferenceData
  {
    std::atomic<size_t> references;

    ReferenceData() : references( 0 ) {}
  };


  class ReferenceCounter
  {
    private:
      ReferenceData* _data;
      mutable std::mutex _mutex;

      void _checkSubtract();

    public:
      // New counter
      ReferenceCounter();

      // Decrement
      ~ReferenceCounter();

      // Copy construct => increment
      ReferenceCounter( const ReferenceCounter& );
      
      // Move construct => invalidate source
      ReferenceCounter( ReferenceCounter&& );

      // Copy assignment => decrement current, increment new
      ReferenceCounter& operator=( const ReferenceCounter& );

      // Move assignment => decrement current, invalidate source
      ReferenceCounter& operator=( ReferenceCounter&& );

      // Return the current number of copies
      size_t getNumber() const;

      // Return true if it is actively counting references
      operator bool() const;
  };

}

#endif // STEWARDESS_REFERENCE_COUNTER_H_

