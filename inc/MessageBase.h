
#ifndef MESSAGE_BASE_H_
#define MESSAGE_BASE_H_

#include "Definitions.h"

#include <chrono>


class Connection;

class MessageBase
{
  private:

  public:
    // Empty Constructor
    MessageBase();

    // Virtual destructor
    virtual ~MessageBase();
};


#endif // MESSAGE_BASE_H_

