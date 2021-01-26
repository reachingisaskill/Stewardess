
#ifndef MANAGER_H_
#define MANAGER_H_

#include <mutex>


// Forward declare the connection struct so we can hide the backend PImpl style.
struct Management;

// Define the main interface
class Manager
{
  private:
    // Count the number of managers alive
    static size_t _instanceCount;
    static std::mutex _instanceCountMutex;

    Management* _management;

  public:

    Manager( int );

    ~Manager();

    Manager( const Manager& ) = delete;
    Manager( Manager&& ) = delete;
    Manager& operator=( const Manager& ) = delete;
    Manager& operator=( Manager&& ) = delete;


    // Calls dispatch and starts the event loop
    void run();

};

#endif // MANAGER_H_

