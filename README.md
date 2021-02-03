
# Stewardess

Used to be my test repository for playing with libevent and TCP networking.
It's grown slightly and now it has a some applications (coming soon).

## Introduction

Provides a C++11 style API for the development of TCP servers and clients. The library uses the same interface for both, hence serialization protocols, etc need only be written once an re-used.

Users must define the serialization used, some configuration parameters and a server engine class which inherits from CallbackInterface.
As it is based on libevent applications should be developed as event driven state machines. Simply override the virtual functions and the Stweardess will take care of the rest.

Better documentation will follow at some point in the future, when I get round to it. Hey, I'm a busy man!


