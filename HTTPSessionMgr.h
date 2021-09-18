#pragma once
#include "HTTPSession.h"
#include <map>

class HTTPSessionMgr
{
public:
  static HTTPSessionMgr* Instance();
  bool GET(std::string url, 
           HTTPRequest* payload, 
           HTTPResponse& results, 
           bool jsonify = true);
  bool POST(std::string url, 
            HTTPRequest* payload, 
            HTTPResponse& results, 
            bool jsonify = true);

private:
  void SeparateURLObject(std::string url, std::string& server, std::string& object);
  HTTPSession* GetSession(std::string url);
  HTTPSessionMgr();
  
  std::map<std::string, HTTPSession*> Sessions;
};

