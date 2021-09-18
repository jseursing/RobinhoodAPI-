#pragma once
#include "HTTPPayload.h"
#include <string>
#include <Windows.h>
#include <WinInet.h>

// This class is designed to be a singleton because we only want
// one network session for Robinhood.com.
class HTTPSession
{
public:
  bool IsSessionActive() const;
  bool POST(std::string object, 
            HTTPRequest* payload,
            HTTPResponse& results,
            bool jsonify);
  bool GET(std::string object,
           HTTPRequest* payload,
           HTTPResponse& results,
           bool jsonify);

  HTTPSession(std::string serverURL);
  ~HTTPSession();

private:
  void ReadNetworkData(HINTERNET httpRequest,  
                       std::string& data, 
                       HTTPResponse& responseHeaders);

  // Members
  HINTERNET Session;
  HINTERNET Connection;
  unsigned long SecureFlags;
  const char* AcceptTypes[2] = {"*/*", 0};
};

