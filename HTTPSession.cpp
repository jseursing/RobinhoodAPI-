#include "HTTPSession.h"
#include "RobinAPI.h"
#include <stdio.h>
#pragma comment(lib, "Wininet.lib")


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  IsSessionActive
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPSession::IsSessionActive() const
{
  return ((0 != Connection) && (0 != Session));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  POST
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPSession::POST(std::string object,  
                       HTTPRequest* payload, 
                       HTTPResponse& results,
                       bool jsonify)
{
  // Do not continue if session is not active.
  if (true == IsSessionActive())
  {
    // Initialize new HTTP open request.
    HINTERNET httpRequest = HttpOpenRequestA(Connection,
                                             "POST",         // Verb
                                             object.c_str(), // Object name
                                             "HTTP/1.1",     // Version
                                             0,              // Referrer
                                             AcceptTypes,    // AcceptTypes
                                             SecureFlags,    // Flags
                                             0);             // Context
    if (0 == httpRequest)
    {
      //
      // LOGGER
      //
      if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
      {
        RobinAPI::Log("::POST_HttpOpenRequestA failed\n");
      }

      return false;
    }

    // Build body data.
    std::string requestParams;
    if (0 != payload)
    {
      payload->BuildPOSTRequest(requestParams);
      if ((0 < requestParams.size()) &&
          (0 != requestParams.back()))
      {
        requestParams.back() = 0; // Null-Terminate 
      }
    }

    // Create pointer to body data.
    void* body = reinterpret_cast<void*>(&requestParams[0]);
    long bodyLength = requestParams.length() - 1;
    if (-1 == bodyLength)
    {
      bodyLength = 0;
    }

    // Build Headers data.
    payload->BuildRequestHeaders(httpRequest, bodyLength);

    // Send http request
    if (FALSE == HttpSendRequestA(httpRequest, // Request object
                                  0,           // Headers (prefilled)
                                  0,           // Length of Headers (prefilled)
                                  body,        // Body (in objectParams)
                                  bodyLength)) // Body length
    {
      InternetCloseHandle(httpRequest);

      //
      // LOGGER
      //
      if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
      {
        RobinAPI::Log("::POST_HttpSendRequestA failed\n");
      }

      return false;
    }

    // Fill Payload with response information and retrieve data.
    std::string data;
    ReadNetworkData(httpRequest, data, results);
    InternetCloseHandle(httpRequest);

    // Format payload3
    if (true == jsonify)
    {
      results.JsonifyPayload(data);
    }
    else
    {
      results.SetRawData(data);
    }

    //
    // LOGGER
    //
    if (RobinAPI::LOG_TRAFFIC & RobinAPI::DEBUG_LEVEL)
    {
      std::string output = 
        "::POST " + object + "\n" +
        "--------\nREQUEST\n--------\n" + payload->ToString() + "\n" +
        "--------\nRESPONSE\n--------\n" + results.ToString() + "\n";             
      RobinAPI::Log(output);
    }

    // Return based on response status
    return 0 == results.ErrorStatus.length();
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GET
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPSession::GET(std::string object,
                      HTTPRequest* payload,
                      HTTPResponse& results,
                      bool jsonify)
{
  // Do not continue if session is not active.
  if (true == IsSessionActive())
  {
    // Build body data.
    std::string requestParams;
    if (0 != payload)
    {
      payload->BuildGETRequest(requestParams);
      if ((0 < requestParams.length()) && 
          (0 != requestParams.back()))
      {
        requestParams.back() = 0; // Null-Terminate 
      }
    }

    // Initialize new HTTP open request, append 
    std::string objectParams = object + requestParams;
    HINTERNET httpRequest = HttpOpenRequestA(Connection,
                                             "GET",                // Verb
                                             objectParams.c_str(), // Object name
                                             "HTTP/1.1",           // Version
                                             0,                    // Referrer
                                             AcceptTypes,          // AcceptTypes
                                             SecureFlags,          // Flags
                                             0);                   // Context
    if (0 == httpRequest)
    {
      //
      // LOGGER
      //
      if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
      {
        RobinAPI::Log("::GET_HttpOpenRequestA failed\n");
      }

      return false;
    }

    // Build Headers data.
    payload->BuildRequestHeaders(httpRequest, 0);

    // Send http request
    if (FALSE == HttpSendRequestA(httpRequest, // Request object
                                  0,           // Headers (prefilled)
                                  0,           // Length of Headers (prefilled)
                                  0,           // Body (in objectParams)
                                  0))          // Body length
    {
      InternetCloseHandle(httpRequest);

      //
      // LOGGER
      //
      if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
      {
        RobinAPI::Log("::GET_HttpSendRequestA failed\n");
      }

      return false;
    }

    // Retrieve response header and data.
    std::string data;
    ReadNetworkData(httpRequest, data, results);
    InternetCloseHandle(httpRequest);

    // Format payload3
    if (true == jsonify)
    {
      results.JsonifyPayload(data);
    }
    else
    {
      results.SetRawData(data);
    }

    //
    // LOGGER
    //
    if (RobinAPI::LOG_TRAFFIC & RobinAPI::DEBUG_LEVEL)
    {
      std::string output = 
        "::GET " + object + "\n" +
        "--------\nREQUEST\n--------\n" + payload->ToString() + "\n" +
        "--------\nRESPONSE\n--------\n" + data + "\n";
      RobinAPI::Log(output);
    }

    // Return based on response status
    return 200 == results.Status;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPSession::HTTPSession(std::string serverURL) :
  Session(0),
  Connection(0),
  SecureFlags(INTERNET_FLAG_RELOAD |
              INTERNET_FLAG_NO_CACHE_WRITE |
              INTERNET_FLAG_SECURE |
              INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
              INTERNET_FLAG_IGNORE_CERT_DATE_INVALID)
{
  Session = InternetOpenA("Mozilla/5.0",                // User-Agent
                          INTERNET_OPEN_TYPE_PRECONFIG, // Configuration
                          0, 0, 0);                     // Proxy, Bypass, Flags
  if (0 == Session)
  {
    //
    // LOGGER
    //
    if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
    {
      RobinAPI::Log("::HTTPSession_InternetOpenA failed\n");
    }

    return;
  }

  Connection = InternetConnectA(Session,
                                serverURL.c_str(),           // Server name
                                INTERNET_DEFAULT_HTTPS_PORT, // Server port
                                0, 0,                        // User/Pass
                                INTERNET_SERVICE_HTTP,       // Service
                                0, 0);                       // Flags/Context
  if (0 == Connection)
  {
    //
    // LOGGER
    //
    if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
    {
      RobinAPI::Log("::HTTPSession_InternetConnectA failed\n");
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Destructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPSession::~HTTPSession()
{
  if (0 != Connection)
  {
    InternetCloseHandle(Connection);
    Connection = 0;
  }

  if (0 != Session)
  {
    InternetCloseHandle(Session);
    Session = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  ReadNetworkData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPSession::ReadNetworkData(HINTERNET httpRequest, 
                                  std::string& data,
                                  HTTPResponse& responseHeaders)
{
  // Clear the output parameter.
  data.clear();
  
  // Retrieve the response header information. The first pass should fail
  // due to no buffer allocation. We should only run it at most, twice.
  std::string headerBuf;
  unsigned long headerSize = 0;
  
  for (unsigned int attempt = 0; attempt < 2; ++attempt)
  {
    if (FALSE == HttpQueryInfoA(httpRequest,
                                HTTP_QUERY_RAW_HEADERS_CRLF,
                                &headerBuf[0],
                                &headerSize,
                                0))
    {
      // If no header is found, exit.
      if (ERROR_HTTP_HEADER_NOT_FOUND == GetLastError())
      {
        //
        // LOGGER
        //
        if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
        {
          RobinAPI::Log("::ReadNetworkData_HttpQueryInfoA - Headers not found\n");
        }

        return;
      }

      // We are expecting an error due to insufficient buffer space.
      if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
      {
        headerBuf.resize(headerSize + 1, 0);
        continue;
      }
    }

    // This portion of code expects a successful read.
    responseHeaders.BuildResponseHeaders(headerBuf);
    break;
  }

  // Download all available information.
  while (true)
  {
    // Retrieve total size of downloadable data.
    unsigned long size = 0;
    if (FALSE == InternetQueryDataAvailable(httpRequest, &size, 0, 0))
    {
      //
      // LOGGER
      //
      if (RobinAPI::LOG_ERRORS & RobinAPI::DEBUG_LEVEL)
      {
        RobinAPI::Log("::ReadNetworkData_InternetQueryDataAvailable failed\n");
      }

      break;
    }
      
    // Allocate a new buffer and set to 0
    char* buffer = new char[size + 1];
    memset(buffer, 0, size + 1);

    // Read internet data into buffer, exit if failure is returned.
    unsigned long bytesRead = 0;
    if (FALSE == InternetReadFile(httpRequest, buffer, size, &bytesRead))
    {
      delete[] buffer;
      break;
    }

    // Exit if nothing was downloaded.
    if (0 == bytesRead)
    {
      delete[] buffer;
      break;
    }

    // If the status is 200, add to buffer, otherwise update error status.
    buffer[bytesRead] = 0;

    if ((200 <= responseHeaders.Status) && 
        (299 >= responseHeaders.Status))
    {
      data += buffer;
    }
    else
    {
      responseHeaders.ErrorStatus = buffer;
    }

    // Free memory
    delete[] buffer;
  }
}