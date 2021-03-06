
#ifndef STEWARDESS_MANAGER_H_
#define STEWARDESS_MANAGER_H_

#include "Definitions.h"
#include "Configuration.h"
#include "Handle.h"


namespace Stewardess
{

  class CallbackInterface;
  class ManagerImpl;

  class Manager
  {
    private:
      // Count the number of managers alive
      static size_t _instanceCount;
      static std::mutex _instanceCountMutex;


      // PImpl pattern to abstract the backend
      ManagerImpl* _impl;


    public:
      Manager( const Configuration&, CallbackInterface& );
      virtual ~Manager();

      // Not moveable or copyable. We count instances and avoid data races/segfaults
      Manager( const Manager& ) = delete;
      Manager( Manager&& ) = delete;
      Manager& operator=( const Manager& ) = delete;
      Manager& operator=( Manager&& ) = delete;


      // Return a reference to the callback interface
      CallbackInterface& server();
      const CallbackInterface& server() const;


      // Return the port that we are listening on
      int getPortNumber() const;


      // Return our IP address
      std::string getIPAddress() const;


      // Returns true if everything is to run on the primary thread
      bool singleThreadMode() const;



      // Return the timestamp of the last server tick
      TimeStamp getLastTickTime() const;

      // Return the time the server was started
      TimeStamp getStartTime() const;

      // Return the uptime in seconds
      Seconds getUpTime() const;



      // Makes a new connection to the requested host and port number. Blocks while running.
      //  Optional identifier to give the connection object
      Handle connectTo( std::string, std::string, UniqueID = 0 );

      // Requests a new connection to provided host and port number. Runs asynchronously.
      void requestConnectTo( std::string, std::string, UniqueID = 0 );

      // Returns the number of current active connections
      size_t getNumberConnections() const;


      // Creates a timer for the user to use
      void createTimer( UniqueID, bool );

      // Starts the countdown of a timer
      void startTimerClock( UniqueID, TimeStamp );

      // Starts the countdown of a timer
      void startTimerCountdown( UniqueID, Milliseconds );


      // Runs the server instance
      void run();


      // Triggers the shutdown proceedure manually
      void shutdown();


      // Forces everything to close immediately. Not recommended.
      void abort();
  };

}

#endif // MANAGER_H_

