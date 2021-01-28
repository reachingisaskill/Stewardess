
#ifndef MESSAGE_BUILDER_BASE_H_
#define MESSAGE_BUILDER_BASE_H_

class MessageBase;

class MessageBuilderBase
{
  public:

    // Return a finished message
    virtual MessageBase* getMessage() = 0;

    // Return a flag to indicate that a message is half-built.
    virtual bool isBuilding() const = 0;

    // Return a flag to state that a message is finished. Is called after each character is pushed.
    virtual bool isBuilt() const = 0;

    // Push a character into the processing system
    virtual void build( char ) = 0;

    // Create a clone for each connection
    virtual MessageBuilderBase* clone() const = 0;

    // Declares an error has happened
    virtual bool error() const = 0;

    // Return an error string describing the error
    virtual const char* getError() const = 0;


    // Streamer iterface for simplicity
    // Call build
    void operator<<( char c ) { build( c ); }

    // Call is built
    operator bool() { return isBuilt(); }
};

#endif // MESSAGE_BUILDER_BASE_H_

