
#include "Connection.h"
#include "ConnectionData.h"
#include "Payload.h"
#include "Serializer.h"


size_t Connection::_counter = 0;


Connection::Connection( ConnectionData* d ) :
  _data( d ),
  _idNumber( _counter++ )
{
}


Connection::~Connection()
{
  if ( _data != nullptr )
  {
    delete _data->serializer;
    delete _data;
  }
}

ConnectionData* Connection::getData()
{
  return _data;
}


bool Connection::isOpen() const
{
  return (_data != nullptr) && (!_data->close);
}


void Connection::close() const
{
  _data->close = true;
}


void Connection::write( Payload* p ) const
{
  _data->serializer->serialize( p );
}

