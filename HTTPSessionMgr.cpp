#include "HTTPSessionMgr.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Instance
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPSessionMgr* HTTPSessionMgr::Instance()
{
  static HTTPSessionMgr instance;
  return &instance;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GET
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPSessionMgr::GET(std::string url, 
                         HTTPRequest* payload, 
                         HTTPResponse& results, 
                         bool jsonify)
{
  std::string server;
  std::string object;
  SeparateURLObject(url, server, object);

  HTTPSession* session = GetSession(server);
  if (0 != session)
  {
    return session->GET(object, payload, results, jsonify);
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  POST
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool HTTPSessionMgr::POST(std::string url, 
                          HTTPRequest* payload, 
                          HTTPResponse& results,
                          bool jsonify)
{
  std::string server;
  std::string object;
  SeparateURLObject(url, server, object);

  HTTPSession* session = GetSession(server);
  if (0 != session)
  {
    return session->POST(object, payload, results, jsonify);
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SeparateURLObject
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void HTTPSessionMgr::SeparateURLObject(std::string url, 
                                       std::string& server, 
                                       std::string& object)
{
  // First strip everything before the url...
  std::string realURL = url;
  unsigned int startPos = url.find("//");
  if (std::string::npos != startPos)
  {
    realURL = url.substr(startPos + 2);
  }

  unsigned int pos = realURL.find("/");
  if (std::string::npos != pos)
  {
    server = realURL.substr(0, pos);
    object = realURL.substr(pos);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetSession
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPSession* HTTPSessionMgr::GetSession(std::string url)
{
  HTTPSession* session = 0;
  if (Sessions.end() != Sessions.find(url))
  {
    return Sessions[url];
  }

  session = new HTTPSession(url);
  Sessions[url] = session;
  return session;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  HTTPSessionMgr
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
HTTPSessionMgr::HTTPSessionMgr()
{

}