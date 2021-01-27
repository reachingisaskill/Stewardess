
#include "Connection.h"
#include "ConnectionData.h"


size_t Connection::_counter = 0;


Connection::Connection( ConnectionData* d ) :
  _data( d ),
  _idNumber( _counter++ )
{
}


Connection::~Connection()
{
  delete _data;
}

ConnectionData* Connection::getData()
{
  return _data;
}

