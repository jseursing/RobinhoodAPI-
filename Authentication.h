#pragma once
#include "RobinConstants.h"

class Authentication
{
friend class RobinAPI;

public:
  static Authentication* Instance();
  RobinConstants::StatusType Login(std::string email, 
                                   std::string passw,
                                   std::string mfa_code);
  void RemoveSession(std::string email);

private:
  static void GetRandomDeviceToken(std::string& token);

  Authentication();
};

