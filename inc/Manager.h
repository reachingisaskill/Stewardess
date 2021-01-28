
#ifndef MANAGER_H_
#define MANAGER_H_

#include "Definitions.h"
#include "MessageBuilderBase.h"
#include "Connection.h"
#include "Event.h"
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
    MutexedBuffer<Event> _eventBuffer;

    // Map of all connections
    ConnectionMap _connectionMap; 
    mutable std::mutex _connectionMapMutex;

    // True when there is an active listener object enabled
    bool _isListening;


  protected:


  public:

    Manager( int, MessageBuilderBase* );

    ~Manager();

    Manager( const Manager& ) = delete;
    Manager( Manager&& ) = delete;
    Manager& operator=( const Manager& ) = delete;
    Manager& operator=( Manager&& ) = delete;


    // Start the loop listening for connections
    void run();

    // Stops everything
    void stop();

    // Close the and clear all the events
    void close();

    // Blocks the calling thread until either a read event occurs or some error must be processed
    bool pop( Event& );


    // Push a message onto the message buffer
    void publishEvent( Event );

    // Add a new connection to the connection map
    void addConnection( Connection* );

    // Remove a closed connection
    void removeConnection( Connection* );


    // Returns true if the listener is active
    bool isListening() const { return _isListening; };
};

#endif // MANAGER_H_

