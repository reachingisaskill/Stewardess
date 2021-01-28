
#ifndef MANAGER_H_
#define MANAGER_H_

#include "MessageBuilderBase.h"
#include "Connection.h"
#include "MutexedBuffer.h"

#include <mutex>


// Forward declare the connection struct so we can hide the backend PImpl style.
struct ManagerData;
class Connection;

// Define the main interface
class Manager
{
  private:
    // Count the number of managers alive
    static size_t _instanceCount;
    static std::mutex _instanceCountMutex;

    // Data
    ManagerData* _data;

    // Mutexed message buffer
    MutexedBuffer<MessageBase*> _messageBuffer;


  protected:
    // Add a new connection once the listener has found someone
    void addConnection( Connection* ) {}


  public:

    Manager( int, MessageBuilderBase* );

    ~Manager();

    Manager( const Manager& ) = delete;
    Manager( Manager&& ) = delete;
    Manager& operator=( const Manager& ) = delete;
    Manager& operator=( Manager&& ) = delete;


    // Start the loop listening for connections
    void run();

    // Close the listening loop and clear all the events
    void close();

    // Blocks the calling thread until either a read event occurs or some error must be processed
    bool pop( MessageBase*& );

    // Pushes a message in the send queue.
    void push( MessageBase* );


    // Push a message onto the message buffer
    void publishMessage( MessageBase* );
};

#endif // MANAGER_H_

