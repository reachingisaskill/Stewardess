
#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <cstddef>


class ConnectionData;
class Payload;


class Connection
{
  friend void connectionUpdate( Connection* );

  private:
    static size_t _counter;

    // Hidden connection data
    ConnectionData* _data;

    // "Unique" ID of the connection
    size_t _idNumber;

  public:
    Connection( ConnectionData* );
    ~Connection();

    // Return a pointer to the internal data
    ConnectionData* getData();


    // Return the ID number
    size_t getIDNumber() const { return _idNumber; }

    // Returns true if the connection is still alive
    bool isOpen() const;


    // Force the connection to close at the end of the current exectution
    void close() const;

    // Writes a payload to the output buffer
    void write( Payload* ) const;
};

#endif // CONNECTION_H_

