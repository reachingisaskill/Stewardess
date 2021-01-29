
#ifndef SERIALIZER_BASE_H_
#define SERIALIZER_BASE_H_

#include "Definitions.h"

class Payload;

class Serializer
{
  public:
    virtual ~Serializer() {}

    // Turns a payload into a message for writing
    virtual void serialize( Payload* ) = 0;

    // Turns a payload into a message for writing
    virtual const char* payloadBuffer() = 0;

    // Turns a payload into a message for writing
    virtual size_t payloadBufferSize() = 0;

    // Return a finished message
    virtual Payload* getPayload() = 0;

    // Return a flag to indicate that a message is half-built.
    virtual bool isBuilding() const = 0;

    // Return a flag to state that a message is finished. Is called after each character is pushed.
    virtual bool isBuilt() const = 0;

    // Push a character into the processing system
    virtual void build( char ) = 0;

    // Declares an error has happened
    virtual bool error() const = 0;

    // Return an error string describing the error
    virtual const char* getError() const = 0;
};

#endif // SERIALIZER_BASE_H_

