
#ifndef MESSAGE_BASE_H_
#define MESSAGE_BASE_H_

#include "Definitions.h"

#include <chrono>


class Connection;

class MessageBase
{
  private:
    Connection* _theConnection;
    TimeStamp _timeStamp;


  public:
    // Empty Constructor
    MessageBase();

    // Virtual destructor
    virtual ~MessageBase();


    // Set the associated connection
    void setConnection( Connection* c ) { _theConnection = c; }


    // Return a pointer to the associated connection
    const Connection* getConnection() const { return _theConnection; }

    // Return the creation time
    TimeStamp getTimeStamp() const { return _timeStamp; }
};


#endif // MESSAGE_BASE_H_

