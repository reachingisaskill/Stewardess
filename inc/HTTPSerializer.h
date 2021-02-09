
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
      typedef std::map<std::string, std::string> HeaderData;

      enum MethodType { Response, Get, Post };

      enum ResponseType { Ok = 200, NotFound = 404 };

    private:
      MethodType _method;

      std::string _request;

      std::string _version;

      ResponseType _response;

      HeaderData _header;

      std::string _body;


      void _configureDefaults();

    public:
      HTTPPayload();

      HTTPPayload( ResponseType, std::string );


      void setHeader( std::string, std::string );


      MethodType getMethod() const { return _method; }

      std::string getHeader( std::string ) const;

      const std::string& getBody() const { return _body; }

      std::string print() const;

  };

}

#endif // STEWARDESS_HTTP_PAYLOAD_H_

