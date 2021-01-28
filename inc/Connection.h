
#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <cstddef>


class ConnectionData;


class Connection
{
  // Make it non-copyable
  Connection( const Connection& ) = delete;
  Connection& operator=( const Connection& ) = delete;

  public:
    typedef Connection* pointer;

  private:
    static size_t _counter;

    ConnectionData* _data;

    size_t _idNumber;

  public:
    Connection( ConnectionData* );
    ~Connection();

    ConnectionData* getData();

    // Returns true if the connection is still alive
    bool isOpen() const;
};

#endif // CONNECTION_H_

