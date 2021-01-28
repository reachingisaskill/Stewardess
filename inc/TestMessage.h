
#ifndef TEST_MESSAGE_H_
#define TEST_MESSAGE_H_

#include "MessageBase.h"
#include "MessageBuilderBase.h"

#include <string>


class TestMessage;


class TestMessageBuilder : public MessageBuilderBase
{
  private:
    std::string _currentString;

    bool _isBuilding;
    bool _isBuilt;
    

  public:
    // Basic con/destructors
    TestMessageBuilder();
    virtual ~TestMessageBuilder();

    // Return a finished message
    virtual MessageBase* getMessage();

    // Return a flag to indicate that a message is half-built.
    virtual bool isBuilding() const { return _isBuilding; }

    // Return a flag to state that a message is finished. Is called after each character is pushed.
    virtual bool isBuilt() const { return _isBuilt; }

    // Push a character into the processing system
    virtual void build( char );

    // Return true on error
    virtual bool error() const override { return false; }

    // Return an error string describing the error
    virtual const char* getError() const override { return ""; }

    // Create a clone for each connection
    virtual MessageBuilderBase* clone() const { return (MessageBuilderBase*) new TestMessageBuilder(); }
};



class TestMessage : public MessageBase
{
  private:
    std::string _theMessage;

  public:
    explicit TestMessage( std::string m ) : MessageBase(), _theMessage( m ) {}

    std::string& getMessage() { return _theMessage; }

};

#endif // TEST_MESSAGE_H_

