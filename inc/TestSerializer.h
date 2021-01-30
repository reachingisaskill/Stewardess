
#ifndef TEST_PAYLOAD_H_
#define TEST_PAYLOAD_H_

#include "Definitions.h"
#include "Payload.h"
#include "Serializer.h"


class TestPayload;


class TestSerializer : public Serializer
{
  private:
    // Current message being reconstructed
    std::string _currentPayload;


  public:
    // Basic con/destructors
    TestSerializer();
    virtual ~TestSerializer();


    // Serialize a payload
    virtual void serialize( const Payload* ) override;

    // Turn a character buffer into payload
    virtual void deserialize( const Buffer* ) override;

};



class TestPayload : public Payload
{
  private:
    std::string _theMessage;

  public:
    explicit TestPayload( std::string m ) : Payload(), _theMessage( m ) {}

    const std::string& getMessage() const { return _theMessage; }

};

#endif // TEST_MESSAGE_H_

