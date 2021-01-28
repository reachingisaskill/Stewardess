
#include "MessageBase.h"


MessageBase::MessageBase() :
  _theConnection( nullptr ),
  _timeStamp()
{
  std::time( &_timeStamp );
}


MessageBase::~MessageBase()
{
}

