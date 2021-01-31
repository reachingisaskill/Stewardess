
#ifndef SERVER_MANAGER_H_
#define SERVER_MANAGER_H_

#include "Definitions.h"
#include "ManagerBase.h"

#include <mutex>


// Forward declare the connection struct so we can hide the backend PImpl style.
struct ManagerData;
class Connection;
class CallbackInterface;

// Define the main interface
class ServerManager : public ManagerBase
{
  private:

  protected:
    // Start the loop listening for connections
    void _run( CallbackInterface& ) override;

  public:

    explicit ServerManager( int );

    ~ServerManager();

};

#endif // SERVER_MANAGER_H_

