
#ifndef STEWARDESS_EXCEPTION_H_
#define STEWARDESS_EXCEPTION_H_

#include "Definitions.h"
#include <exception>


namespace Stewardess
{

  class Exception : public std::runtime_error
  {
    private:
      std::string _what;

    public:
      explicit Exception( const char*  );
      explicit Exception( const std::string& );

      virtual const char* what() const noexcept override;
  };

}

#endif // STEWARDESS_EXCEPTION_H_

