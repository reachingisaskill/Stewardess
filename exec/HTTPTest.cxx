
#include "HTTPSerializer.h"
#include "Buffer.h"

#include <iostream>
#include <cassert>

using namespace Stewardess;


int main( int, char** )
{
  {
    HTTPSerializer serialSender;
    HTTPSerializer serialReceiver;

    std::cout << "Creating test payload" << std::endl;
    HTTPPayload* test_payload = new HTTPPayload( HTTPPayload::Ok, "Hello" );
    test_payload->setHeader( "Host", "www.example.com" );

    std::cout << '\n' << test_payload->print() << '\n' << std::endl;

    std::cout << "Serializing" << std::endl;
    serialSender.serialize( test_payload );

    assert( serialSender.bufferEmpty() == false );
    assert( serialSender.errorEmpty() == true );

    std::cout << "Pop buffer" << std::endl;
    Stewardess::Buffer* temp_buffer = serialSender.getBuffer();

    std::cout << "Raw Buffer : " << temp_buffer->getString() << std::endl;

    std::cout << "Deserializing" << std::endl;
    serialReceiver.deserialize( temp_buffer );

    assert( serialReceiver.payloadEmpty() == false );
    assert( serialReceiver.errorEmpty() == true );

    std::cout << "Recovered payload:" << std::endl;

    HTTPPayload* new_payload = (HTTPPayload*)serialReceiver.getPayload();
    std::cout << '\n' << new_payload->print() << '\n' << std::endl;

    delete test_payload;
    delete temp_buffer;
    delete new_payload;
  }

  std::cout << "\n----------------------------------------------------------------------------------------------------\n" << std::endl;

  {
    HTTPSerializer serialSender;
    HTTPSerializer serialReceiver;

    std::cout << "Creating test payload" << std::endl;
    HTTPPayload* test_payload = new HTTPPayload( HTTPPayload::NotImplemented, "Oh balls... \n\n\nShould fix that really..." );
    test_payload->setHeader( "Host", "www.example.com" );

    std::cout << '\n' << test_payload->print() << '\n' << std::endl;

    std::cout << "Serializing" << std::endl;
    serialSender.serialize( test_payload );

    assert( serialSender.bufferEmpty() == false );
    assert( serialSender.errorEmpty() == true );

    std::cout << "Pop buffer" << std::endl;
    Stewardess::Buffer* temp_buffer = serialSender.getBuffer();

    std::cout << "Deserializing" << std::endl;
    serialReceiver.deserialize( temp_buffer );

    assert( serialReceiver.payloadEmpty() == false );
    assert( serialReceiver.errorEmpty() == true );

    std::cout << "Recovered payload:" << std::endl;

    HTTPPayload* new_payload = (HTTPPayload*)serialReceiver.getPayload();
    std::cout << '\n' << new_payload->print() << '\n' << std::endl;

    delete test_payload;
    delete temp_buffer;
    delete new_payload;
  }

  return 0;
}


