#include "Authentication.h"
#include "HTTPSessionMgr.h"
#include "RobinAPI.h"
#include <chrono>
#include <direct.h>
#include <fstream>
#include <random>
#include <Shlwapi.h>

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Instance
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Authentication* Authentication::Instance()
{
  static Authentication instance;
  return &instance;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Login
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Authentication::Login(std::string email,
                                                 std::string passw,
                                                 std::string mfa_code)
{
  // Local members
  std::string output;

  // Generate a random device token
  std::string deviceToken;
  GetRandomDeviceToken(deviceToken);
  RobinAPI::SetDeviceToken(deviceToken.c_str());

  // Validate email and password before continuing.
  if ((0 == email.length()) ||
      (0 == passw.length()))
  {
    return RobinConstants::STATUS_BAD_CREDENTIALS;
  }

  // Construct our payload (body)
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);
  payload.Add("client_id", "c82SH0WZOsabOXGP2sxqcj34FxkvfnWRZBKlBjFS"); // ???
  payload.Add("expires_in", "86400"); // 12-Hours
  payload.Add("grant_type", "password");
  payload.Add("password", passw);
  payload.Add("scope", "internal");
  payload.Add("username", email);
  payload.Add("challenge_type", "sms");
  payload.Add("device_token", deviceToken);

  // If mfa_code is specified, add it to the payload.
  if (0 < mfa_code.length())
  {
    payload.Add("mfa_code", mfa_code); 
  }

  // Post a request
  HTTPResponse response;
  HTTPSessionMgr::Instance()->POST(RobinConstants::LoginURL(), &payload, response);
  if (400 == response.Status)
  {
    return RobinConstants::STATUS_BAD_CREDENTIALS;
  }
  else if (200 != response.Status)
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Check for "detail" key.
  if (-1 != response.Contains("detail"))
  {
    // This detail specifies invalid 2fa.
    if (std::string::npos != response.Get("detail").find("Please enter a valid"))
    {
      return RobinConstants::STATUS_BAD_2FA;
    }
  }

  // Check for "mfa_required" and if it is set to true, alert the user.
  if (-1 != response.Contains("mfa_required"))
  {
    if (0 == response.Get("mfa_required").compare("true"))
    {
      return RobinConstants::STATUS_2FA;
    }
  }

  // We are going to assume we received an authorization token at this point.
  // verify this assumption and store it.
  if (-1 != response.Contains("access_token"))
  {
    // Update Email and other members
    RobinAPI::Email = email;
    RobinAPI::SetTokenType(response.Get("token_type").c_str());
    RobinAPI::SetAccessToken(response.Get("access_token").c_str());
    RobinAPI::SetRefreshToken(response.Get("refresh_token").c_str());

    // Update HTTP Headers
    RobinAPI::EmptyRequest->AddToHeader("Authorization", RobinAPI::GetAuthorization());

    // Return status
    return RobinConstants::STATUS_OK;
  }

  return RobinConstants::UNKNOWN_STATE;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetRandomDeviceToken
//  Notes:     Generates a random device token in the form of a string:
//             ie: 16c73a2a-80d1-2c55-051b-73c03bb20e96
//                 62271f29-f601-b117-9549-6017ed5e52d4
//
/////////////////////////////////////////////////////////////////////////////////////////
void Authentication::GetRandomDeviceToken(std::string& token)
{
  // Token character map
  static char TOKEN_KEY_MAP[] = 
  {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
  };

  // Initialize random device based on current time.
  std::random_device device;
  std::mt19937::result_type seed = 
    device() ^ static_cast<std::mt19937::result_type>
               (std::chrono::duration_cast<std::chrono::seconds>
               (std::chrono::system_clock::now().time_since_epoch()).count());
  std::mt19937 randGenerator(seed);
  std::uniform_int_distribution<int> distribution(0, sizeof(TOKEN_KEY_MAP) - 1);

  // Initialize token output
  token.resize(RobinAPI::DEVICE_TOKEN_LEN);

  // Fill in destination buffer with random characters
  for (unsigned int i = 0; i < RobinAPI::DEVICE_TOKEN_LEN; ++i)
  {
    // Place a - in the token for the following positions,
    // otherwise fill with a random character...
    switch (i)
    {
      case 8:
      case 13:
      case 18:
      case 23:
        token[i] = '-';
        break;
      default:
        token[i] = TOKEN_KEY_MAP[distribution(randGenerator)];
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RemoveSession
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void Authentication::RemoveSession(std::string email)
{
  // Convert email to hash
  std::string emailHash = std::to_string(std::hash<std::string>{}(email));

  // Retrieve current directory, append hash
  std::string sessionPath(256, 0);
  HMODULE thisModule = GetModuleHandle(0);
  GetModuleFileNameA(thisModule, &sessionPath[0], sessionPath.length());
  sessionPath = sessionPath.substr(0, sessionPath.find_last_of("\\") + 1);
  sessionPath += emailHash;

  // Attempt to open the filename using hash, delete if it exists.
  std::ifstream iStream(sessionPath);
  if (true == iStream.is_open())
  {
    // Update Email
    RobinAPI::Email.clear();

    // Close file
    iStream.close();
    remove(sessionPath.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Authentication::Authentication()
{

}