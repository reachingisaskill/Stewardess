
# Stewardess

## Introduction

An asynchronous Application Layer library that provides OOP interfaces for the use of custom protocols between TCP devices.
It uses libevent to wait on file descriptor events as it choose the most appropriate backend for each system. (e.g. select, epoll, etc)
I chose not to use the bufferevent structure, rather I implemented a custom buffering object allowing more C++ friendly functions such as streaming operators and the implementation of write queue.
The number of threads is variable at runtime and each thread is given a libevent event_base to handle its own connections.
The whole system was designed to be completely asynchronous regardless of the actions of user. Any thread may affect any connection without fear of data races or corruption. However the server callbacks are also asynchronous so you must be careful in the server logic!


## Asynchronous you say?

The system expects the user to configure a number of worker threads. Each established connected is accessed only within its assigned thread, however by using the connection Handle objects any thread may queue messages to or inspect a connection.

The Handles are reference counted and expected to be stored for systems that leave the connection open. The use case is that for each server event the user has the option of writing to any of the connections currently open regardless of whether the connection was established by that thread. The system also ensures that connections are closed under a mutex, but the connection data is not deallocated until all the Handles have been destroyed, preventing accidental segment violations. 


## Basic configuration

To get a usable system you must:
1. Define a protocol. This involves implmenting a Payload derived class and a Serializer derived class. The payload should be serialised (converted to a byte buffer) and deserialised by the serialiser.
2. Implement a CallBackInterface. This is essentially the server logic. Users can implment any or none of the event callbacks e.g. onStart(), onRead(), onEvent() etc. There is additionally an onTick() function called at a configurable frequency.
3. Set the configuration. The comfiguration object should be created and filled with the settings needed including options such as a ListernerEvent to accept incoming connects, the port being used or the number of threads.

Once everything is setup the Manager object loads the configuration, configures the event_base's and initialises the the system, calling the appropriate callback functions along the way.

## TODO List

- Move the connection map from the manager to the respective thread handlers. Each thread closes its own connections anyway, so this would reduce the size of the map that stores them and increased lookup speed and reduce mutex contension.

- Finish the SSL implmentation. It will be added as a configuration option (along with certificate files, etc) and transparent to ther user.
