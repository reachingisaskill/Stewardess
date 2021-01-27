
#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <memory>
#include <string>


class Connection;

class Message
{
  public:
    typedef std::unique_ptr<Message> pointer;

  public:
    Connection* _theConnection;
    std::string _theMessage;

  public:
    Message() : _theConnection( nullptr ), _theMessage() {}

    Message( Connection* c, std::string s = "" ) :
      _theConnection( c ),
      _theMessage( s ) {}
};


#endif // MESSAGE_H_

