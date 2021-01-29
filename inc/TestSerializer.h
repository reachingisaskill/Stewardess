
#ifndef TEST_PAYLOAD_H_
#define TEST_PAYLOAD_H_

#include "Definitions.h"
#include "Payload.h"
#include "Serializer.h"


class TestPayload;


class TestSerializer : public Serializer
{
  private:
    std::string _currentString;
    std::vector< char > _buffer;

    bool _isBuilding;
    bool _isBuilt;
    

  public:
    // Basic con/destructors
    TestSerializer();
    virtual ~TestSerializer();

    // Serialize a payload
    virtual void serialize( Payload* p ) override;

    // Turns a payload into a message for writing
    virtual const char* payloadBuffer() override;

    // Turns a payload into a message for writing
    virtual size_t payloadBufferSize() override;


    // Return a finished message
    virtual Payload* getPayload() override;

    // Return a flag to indicate that a message is half-built.
    virtual bool isBuilding() const override { return _isBuilding; }

    // Return a flag to state that a message is finished. Is called after each character is pushed.
    virtual bool isBuilt() const override { return _isBuilt; }

    // Push a character into the processing system
    virtual void build( char ) override;

    // Return true on error
    virtual bool error() const override { return false; }

    // Return an error string describing the error
    virtual const char* getError() const override { return ""; }
};



class TestPayload : public Payload
{
  private:
    std::string _theMessage;

  public:
    explicit TestPayload( std::string m ) : Payload(), _theMessage( m ) {}

    std::string& getMessage() { return _theMessage; }

};

#endif // TEST_MESSAGE_H_

