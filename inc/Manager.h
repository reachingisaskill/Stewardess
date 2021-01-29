
#ifndef MANAGER_H_
#define MANAGER_H_

#include "Definitions.h"

#include <mutex>


// Forward declare the connection struct so we can hide the backend PImpl style.
struct ManagerData;
class Connection;
class ServerState;

// Define the main interface
class Manager
{
  private:
    // Count the number of managers alive
    static size_t _instanceCount;
    static std::mutex _instanceCountMutex;

    // Data
    ManagerData* _data;

  public:

    Manager( int );

    ~Manager();

    Manager( const Manager& ) = delete;
    Manager( Manager&& ) = delete;
    Manager& operator=( const Manager& ) = delete;
    Manager& operator=( Manager&& ) = delete;


    // Set the number of connection handling threads
    void setNumberThreads( unsigned );


    // Start the loop listening for connections
    void run( ServerState& );


    // Returns a reference to the current server state object
    ServerState& getServerState() const;

};

#endif // MANAGER_H_

