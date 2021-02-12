
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
    typedef std::map<std::string, std::string> HeaderData;

    public:

      enum MethodType { Response, Get, Head, Post, Put, Delete, Trace, Options, Connect, Patch };

      enum ResponseType
      {
        Null = 0,
        Ok = 200,
        Created = 201,
        Accepted = 202,
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HTTPVersionNotSupported = 505
      };

    private:
      // Enumeration of the method
      MethodType _method;

      // The request string if it is a request
      std::string _request;

      // The version string
      std::string _version;

      // The type of the response if it is a response
      ResponseType _response;

      // The header data
      HeaderData _header;

      // If true, load the body data from the file
      bool _isFile;

      // The body data
      std::string _body;


    public:
      // Empty constructor - defaults to a null response
      HTTPPayload();

      // Simple response constructor
      HTTPPayload( ResponseType, std::string );


      // Set the flag to indicate it is a file type payload
      void setFile( bool f ) { _isFile = f; }


      // Set a key-value pair in the header
      void setHeader( std::string, std::string );


      // Return the method of payload
      MethodType getMethod() const { return _method; }

      // Return a requested header value. Returns an empty string if it doesn't exist
      std::string getHeader( std::string ) const;

      // Return the literal body contents
      const std::string& getBody() const { return _body; }

      // For debugging
      std::string print() const;

  };

}

#endif // STEWARDESS_HTTP_PAYLOAD_H_

