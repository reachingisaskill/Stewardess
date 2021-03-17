
#ifndef STEWARDESS_CONNECTION_REQUEST_H_
#define STEWARDESS_CONNECTION_REQUEST_H_

#include "Definitions.h"


namespace Stewardess
{

  struct ConnectionRequest
  {
    std::string address;
    std::string port;
    UniqueID uniqueId;
  };

}

#endif // STEWARDESS_CONNECTION_REQUEST_H_

