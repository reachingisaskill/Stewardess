
#include "Serializer.h"
#include "Payload.h"
#include "Buffer.h"


namespace Stewardess
{

  Serializer::~Serializer()
  {
    while ( ! _payloads.empty() )
    {
      delete _payloads.front();
      _payloads.pop();
    }
    while ( ! _buffers.empty() )
    {
      delete _buffers.front();
      _buffers.pop();
    }
    while ( ! _errors.empty() )
    {
      _errors.pop();
    }
  }


  void Serializer::pushPayload( Payload* p )
  {
    GuardLock lk( _payloadMutex );
    _payloads.push( p );
  }


  void Serializer::pushBuffer( Buffer* b )
  {
    GuardLock lk( _bufferMutex );
    _buffers.push( b );
  }


  void Serializer::pushError( const char* e )
  {
    GuardLock lk( _errorMutex );
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

}

