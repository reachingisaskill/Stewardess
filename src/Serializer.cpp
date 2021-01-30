
#include "Serializer.h"


void Serializer::pushPayload( Payload* p )
{
  _payloads.push( p );
}


void Serializer::pushBuffer( Buffer* b )
{
  _buffers.push( b );
}


void Serializer::pushError( const char* e )
{
  _errors.push( e );
}


Buffer* Serializer::getBuffer()
{
  Buffer* temp = _buffers.front();
  _buffers.pop();
  return temp;
}


bool Serializer::bufferEmpty() const
{
  return _buffers.empty();
}


Payload* Serializer::getPayload()
{
  Payload* p = _payloads.front();
  _payloads.pop();
  return p;
}


bool Serializer::payloadEmpty() const
{
  return _payloads.empty();
}


const char* Serializer::getError()
{
  const char* temp = _errors.front();
  _errors.pop();
  return temp;
}


bool Serializer::errorEmpty() const
{
  return _errors.empty();
}
