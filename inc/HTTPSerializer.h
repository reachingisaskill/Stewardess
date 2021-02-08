
#ifndef STEWARDESS_HTTP_PAYLOAD_H_
#define STEWARDESS_HTTP_PAYLOAD_H_

#include "Definitions.h"
#include "Payload.h"
#include "Serializer.h"


namespace Stewardess
{

  class HTTPPayload;


  class HTTPSerializer : public Serializer
  {
    private:

    public:
      // Basic con/destructors
      HTTPSerializer() {}
      virtual ~HTTPSerializer() {}


      // Serialize a payload
      virtual void serialize( const Payload* ) override;

      // Turn a character buffer into payload
      virtual void deserialize( const Buffer* ) override;

  };



  class HTTPPayload : public Payload
  {
    friend class HTTPSerializer;

    public:
      typedef std::pair< std::string, std::string > HeaderField;
      typedef std::vector<HeaderFields> HeaderData;

      enum Method { Response, Get, Post };

      enum Response { Ok, NotFound };

    private:
      Method _method;

      std::string _request;

      Response _response;

      HeaderData _header;

      std::string _body;


      void _configureDefaults();

    public:
      HTTPPayload();

      HTTPPayload( std::string, std::string );


      Method getMethod() const { return _method; }

      const HeaderData& getHeader() const { return _header; }

      const std::string& getBody() const { return _body; }

  };

}

#endif // STEWARDESS_HTTP_PAYLOAD_H_

