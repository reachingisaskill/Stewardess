
#ifndef MANAGER_BASE_H_
#define MANAGER_BASE_H_

#include "Definitions.h"


class ManagerData;
class CallbackInterface;


class ManagerBase
{
  private:
    // Count the number of managers alive
    static size_t _instanceCount;
    static std::mutex _instanceCountMutex;

  protected:
    ManagerData* _data;


    virtual void _run( CallbackInterface& ) = 0;

  public:
    ManagerBase();
    virtual ~ManagerBase();

    ManagerBase( const ManagerBase& ) = delete;
    ManagerBase( ManagerBase&& ) = delete;
    ManagerBase& operator=( const ManagerBase& ) = delete;
    ManagerBase& operator=( ManagerBase&& ) = delete;


    // Set the number of connection handling threads
    void setNumberThreads( unsigned );


    // Set the default buffer size. Should probably be bigger than the expected payload size
    void setDefaultBufferSize( size_t );


    // Set the timeouts for each connection
    void setReadTimeout( unsigned int );
    void setWriteTimeout( unsigned int );


    // Set the modifier factor for the internal tick time
    void setTickTimeModifier( float );

    // Set the time before the connections are forcibly closed once shutdown has been called.
    void setDeathTime( unsigned int );


    // Set where the manager can signal connection close when shutdown is called.
    // Note the connection will only close once another event has triggered. e.g. read timeout.
    void setCloseConnectionsOnShutdown( bool );


    // Returns a reference to the current server state object
    CallbackInterface& getCallbackInterface() const;


    // Start the manager and run the libevent loops
    void run( CallbackInterface& );
};


#endif // MANAGER_BASE_H_

