#include "Authentication.h"
#include "HTTPSessionMgr.h"
#include "RobinAPI.h"
#include "Crypto.h"
#include "Stock.h"
#include <chrono>
#include <ctime>
#include <direct.h>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>

//
// Static members
//

// Debug
RobinAPI::DebugType RobinAPI::DEBUG_LEVEL = RobinAPI::NO_LOG;

// Members
bool RobinAPI::LoggedIn = false;
std::string RobinAPI::Path = "";
std::string RobinAPI::Email = "";
std::string RobinAPI::Passw = "";
std::string RobinAPI::DeviceToken = "";
std::string RobinAPI::TokenType = "";
std::string RobinAPI::AccessToken = "";
std::string RobinAPI::RefreshToken = "";
void* RobinAPI::AlertCallback = nullptr;
std::vector<Asset::Listing> RobinAPI::PendingShares;
std::vector<RobinAPI::PriceAlertEntry> RobinAPI::PriceAlerts;
std::vector<RobinAPI::MarketOrder> RobinAPI::MarketOrders;
HTTPRequest* RobinAPI::EmptyRequest = nullptr;
HTTPRequest* RobinAPI::GenericRequest = nullptr;

Portfolio* RobinAPI::ThisPortfolio = 0;
float RobinAPI::VolatileDelta = 0.10f;
unsigned int RobinAPI::VolatileLimit = 2;


/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Log
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::Log(std::string data)
{
  static bool firstLog = true;
  static std::ofstream oStream;

  if (true == firstLog)
  {
    std::string sessionPath = Path + "RobinAPI.log";
    oStream.open(sessionPath);
    firstLog = false;
  }
  
  if (true == oStream.is_open())
  {
    std::time_t result = std::time(0);

    char timestamp[256] = {0};
    std::string strTimestamp = std::asctime(std::localtime(&result));
    strTimestamp[strTimestamp.length()-2] = 0; // Null-Terminate
    sprintf_s(timestamp, "\n[%s] ", strTimestamp.c_str());

    // Write the data to file
    oStream.write(timestamp, strlen(timestamp));
    oStream.write(data.c_str(), data.length());
    oStream.flush();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Initialize
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::Initialize()
{
  // Get current path
  HMODULE thisModule = GetModuleHandle(0);
  Path.resize(MAX_PATH, 0);
  GetModuleFileNameA(thisModule, &Path[0], Path.length());
  Path = Path.substr(0, Path.find_last_of("\\") + 1);

  // If portfolio is already initialized, free it
  if (0 != ThisPortfolio)
  {
    delete ThisPortfolio;
    ThisPortfolio = 0;
  }

  // Initialize portfolio
  ThisPortfolio = new Portfolio();
  if (0 == ThisPortfolio)
  {
    return false;
  }

  // Initialize EmptyRequest
  EmptyRequest = new HTTPRequest();
  EmptyRequest->AddToHeader("Accept-Language", "en-US,en;q=1");
  EmptyRequest->AddToHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
  EmptyRequest->AddToHeader("X-Robinhood-API-Version", "1.315.0");
  EmptyRequest->AddToHeader("Connection", "keep-alive");
  EmptyRequest->AddToHeader("User-Agent", "*");

  // Initialize GenericRequest
  GenericRequest = new HTTPRequest();
  GenericRequest->AddToHeader("Accept-Language", "en-US,en;q=1");
  GenericRequest->AddToHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
  GenericRequest->AddToHeader("Connection", "keep-alive");
  GenericRequest->AddToHeader("User-Agent", "*");

  // Load our database first, output an error if failed.
  if (false == Crypto::LoadDatabase())
  {
    MessageBoxA(0, "Failed retrieving cryptocurrency database.",
                "Warning", MB_ICONERROR);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  IsLoggedIn
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::IsLoggedIn()
{
  return LoggedIn;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetEmail
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetEmail()
{
  return Email;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  IsLoggedIn
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetEmail(const char* value)
{
  Email = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPassword
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetPassword()
{
  return Passw;;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  IsLoggedIn
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetPassword(const char* value)
{
  Passw = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetDeviceToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetDeviceToken()
{
  return DeviceToken;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetDeviceToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetDeviceToken(const char* value)
{
  DeviceToken = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetTokenType
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetTokenType(const char* value)
{
  TokenType = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetTokenType
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetTokenType()
{
  return TokenType;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetAccessToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetAccessToken(const char* value)
{
  AccessToken = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAccessToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetAccessToken()
{
  return AccessToken;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetRefreshToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetRefreshToken(const char* value)
{
  RefreshToken = value;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetRefreshToken
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string& RobinAPI::GetRefreshToken()
{
  return RefreshToken;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAuthorization
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinAPI::GetAuthorization()
{
  std::string auth = TokenType + " " + AccessToken;
  return auth;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  InitiateLogin
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType RobinAPI::InitiateLogin(const char* email,
                                                   const char* passw,
                                                   const char* mfa_code)
{
  RobinConstants::StatusType status = Authentication::Instance()->Login(email, 
                                                                        passw, 
                                                                        mfa_code);
  if (RobinConstants::STATUS_OK == status)
  {
    LoggedIn = true;
    Email = email;
    Passw = passw;
  }

  return status;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  TestAuthorization
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::TestAuthorization()
{
  // Update HTTP Headers
  RobinAPI::EmptyRequest->AddToHeader("Authorization", RobinAPI::GetAuthorization());

  // Build portfolio profile, if status isn't okay, our credentials are bad.
  RobinConstants::StatusType status = ThisPortfolio->BuildPortfolioProfile();
  LoggedIn = (RobinConstants::STATUS_OK == status);
  if (false == LoggedIn)
  {
    RobinAPI::EmptyRequest->RemoveFromHeader("Authorization");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  LogOut
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::InitiateLogOut(bool removeSession)
{
  // Remove authorization from headers
  EmptyRequest->RemoveFromHeader("Authorization");

  // Update login state
  LoggedIn = false;
  Email = "";
  Passw = "";

  // Remove session
  if (true == removeSession)
  {
    Authentication::Instance()->RemoveSession(Email);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  InitiateLogin
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetAuthorization(const char* deviceToken,
                                const char* tokenType,
                                const char* accessToken,
                                const char* refreshToken)
{
  DeviceToken = deviceToken;
  TokenType = tokenType;
  AccessToken = accessToken;
  RefreshToken = refreshToken;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildPortfolioProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildPortfolioProfile()
{
  // Do not continue if we are not logged in.
  if (false == LoggedIn)
  {
    return false;
  }

  // Build portfolio profile
  RobinConstants::StatusType status = ThisPortfolio->BuildPortfolioProfile();
  if (RobinConstants::STATUS_OK != status)
  {
    if (RobinConstants::STATUS_LOGGED_OUT == status)
   {
      InitiateLogOut();
    }

    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildAccountProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildAccountProfile()
{
  // Do not continue if we are not logged in.
  if (false == LoggedIn)
  {
    return false;
  }

  // Build portfolio profile
  RobinConstants::StatusType status = ThisPortfolio->LoadAccountProfile();
  if (RobinConstants::STATUS_LOGGED_OUT == status)
  {
    InitiateLogOut();

    return false;
  }

  return RobinConstants::STATUS_OK == status;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildAssetsProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildAssetsProfile()
{
  // Do not continue if we are not logged in.
  if (false == LoggedIn)
  {
    return false;
  }

  // Retrieve owned stocks
  RobinConstants::StatusType status = Stock::BuildPositions();
  if (RobinConstants::STATUS_OK != status)
  {
    if (RobinConstants::STATUS_LOGGED_OUT == status)
    {
      InitiateLogOut();
    }

    return false;
  }

  // Retrieve owned crypto
  status = Crypto::BuildPositions();
  if (RobinConstants::STATUS_OK != status)
  {
    if (RobinConstants::STATUS_LOGGED_OUT == status)
    {
      InitiateLogOut();
    }

    return false;
  }

  // If there are any pending assets, add them now
  while (0 != PendingShares.size())
  {
    if (-1 == ThisPortfolio->GetAssetBySymbol(PendingShares.back().symbol))
    {
      if (true == PendingShares.back().isCrypto)
      {
        AddCryptoBySymbol(PendingShares.back().symbol);
      }
      else
      {
        AddStock(PendingShares.back().symbol);
      }
    }

    PendingShares.pop_back();
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildHistoricalProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildHistoricalProfile(RobinConstants::Span span, 
                                      RobinConstants::Interval interval)
{
  // Do not continue if we are not logged in.
  if (false == LoggedIn)
  {
    return false;
  }

  // Build portfolio profile
  RobinConstants::StatusType status = ThisPortfolio->BuildHistoricalProfile(span, 
                                                                            interval);
  if (RobinConstants::STATUS_LOGGED_OUT == status)
  {
    InitiateLogOut();

    return false;
  }

  return RobinConstants::STATUS_OK == status;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPortfolioHistory
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Portfolio::HistoricalEntry>* RobinAPI::GetPortfolioHistory()
{
  return ThisPortfolio->GetHistory();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetEquity
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinAPI::GetEquity()
{
  return ThisPortfolio->GetEquity();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPrevClose
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinAPI::GetPrevClose()
{
  return ThisPortfolio->GetPrevClose();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetBuyingPower
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinAPI::GetBuyingPower()
{
  return ThisPortfolio->GetBuyingPower();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetCurrencyCode
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinAPI::GetCurrencyCode()
{
  return ThisPortfolio->GetCurrencyCode();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddOwnedStock
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::AddOwnedStock(std::string url, float quantity, float averagePrice)
{
  ThisPortfolio->AddStock(url, quantity, averagePrice);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetStockShortData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
size_t RobinAPI::GetStockShortData(const char* symbol, std::vector<ShortedStock>& data)
{
  enum
  {
    FEE_IDX = 0,
    TIME_IDX = 1,
    AVAIL_IDX = 2,
    ENTRY_LEN = 3
  };

  data.clear();

  // Send GET request
  HTTPResponse response;
  std::string url = "iborrowdesk.com/api/ticker/" + std::string(symbol);
  if (false != HTTPSessionMgr::Instance()->GET(url, GenericRequest, response))
  {
    int index = response.Contains("daily", HTTPPayload::KeyVal::KEY);
    if (-1 != index)
    {
      bool start = false;
      std::vector<HTTPPayload::KeyVal> payload = response.GetPayload();
      for (int i = index + 1; i < payload.size() - ENTRY_LEN; i += ENTRY_LEN)
      {
        if (false == start)
        {
          // Recent data starts with empty timestamp...
          if (0 == payload[i + TIME_IDX].val.length())
          {
            start = true;
          }

          continue;
        }

        data.push_back({ payload[i + AVAIL_IDX].val,
                         payload[i + FEE_IDX].val, 
                         payload[i + TIME_IDX].val });
      }
    }
  }

  return data.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddStock
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::AddStock(std::string symbol)
{
  return ThisPortfolio->AddStock(symbol.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddCryptoBySymbol
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::AddCryptoBySymbol(std::string symbol)
{
  return ThisPortfolio->AddCrypto(symbol.c_str(), 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddCryptoBySymbol
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::AddOwnedCrypto(std::string symbol, float quantity, float averagePrice)
{
  return ThisPortfolio->AddCrypto(symbol.c_str(), quantity, averagePrice);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAssets
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Asset*>& RobinAPI::GetAssets()
{
  return ThisPortfolio->GetAssets();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAssets
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::AddPendingShare(Asset::Listing& asset)
{
  PendingShares.push_back(asset);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RemoveAsset
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::RemoveAsset(std::string symbol)
{
  return ThisPortfolio->RemoveAsset(symbol);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildAssetsHistoricalData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildAssetsHistoricalData(unsigned int index, 
                                         RobinConstants::Span span, 
                                         RobinConstants::Interval interval)
{

  std::vector<Asset*> assets = ThisPortfolio->GetAssets();
  if (index < assets.size())
  {
    RobinConstants::StatusType status = assets[index]->BuildHistoricalData(span, 
                                                                           interval);
    if (RobinConstants::STATUS_OK == status)
    {
      return true;
    }

    if (RobinConstants::STATUS_LOGGED_OUT == status)
    {
      InitiateLogOut();
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPortfolioHistory
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Asset::HistoricalEntry>* RobinAPI::GetAssetHistory(unsigned int index)
{
  std::vector<Asset*> assets = ThisPortfolio->GetAssets();
  if (index < assets.size())
  {
    return assets[index]->GetHistory();
  }

  return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetLatestAssetPrice
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
float RobinAPI::GetLatestAssetPrice(std::string symbol, bool crypto)
{
  std::string price;
  RobinConstants::StatusType status;
  if (crypto)
  {
    Crypto crypto("", "", symbol, 0.0f, 0.0f);
    status = crypto.Refresh();
    price = crypto.GetLastTradePrice();
  }
  else
  {
    Stock stock("", "", symbol, "", 0.0f, 0.0f);
    status = stock.Refresh();
    price = stock.GetLastTradePrice();
  }

  return (RobinConstants::STATUS_OK == status ? stof(price) : 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPriceAlert
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinAPI::PriceAlertEntry* RobinAPI::GetPriceAlert(std::string symbol)
{
  PriceAlertEntry* entry = nullptr;
  for (size_t i = 0; i < PriceAlerts.size(); ++i)
  {
    if (0 == symbol.compare(PriceAlerts[i].symbol))
    {
      entry = &(PriceAlerts[i]);
      break;
    }
  }

  return entry;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetPriceAlert
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetPriceAlert(std::string symbol, float high, float low)
{
  bool found = false;
  for (size_t i = 0; i < PriceAlerts.size(); ++i)
  {
    if (0 == symbol.compare(PriceAlerts[i].symbol))
    {
      PriceAlerts[i].min = high;
      PriceAlerts[i].max = low;
      PriceAlerts[i].minReady = true;
      PriceAlerts[i].maxReady = true;
      found = true;
      break;
    }
  }

  if (false == found)
  {
    PriceAlertEntry entry;
    entry.symbol = symbol;
    entry.min = high;
    entry.max = low;
    entry.minReady = true;
    entry.maxReady = true;
    PriceAlerts.emplace_back(entry);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetAlertCallback
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetAlertCallback(void* fn)
{
  AlertCallback = fn;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetVolatilityRatio
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetVolatilityRatio(float ratio)
{
  VolatileDelta = ratio;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetVolatilityRatio
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
float RobinAPI::GetVolatilityRatio()
{
  return VolatileDelta;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SetVolatileLimit
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::SetVolatileLimit(unsigned int count)
{
  VolatileLimit = count;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetVolatilityLimit
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int RobinAPI::GetVolatilityLimit()
{
  return VolatileLimit;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchStocks
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int RobinAPI::SearchStocks(std::string query, 
                                    std::vector<Asset::Listing>& results)
{
  // Build request payload
  HTTPRequest payload;
  payload.Add("query", query);

  // Send GET query
  HTTPResponse response;
  if (RobinConstants::STATUS_OK != 
      HTTPSessionMgr::Instance()->GET(RobinConstants::InstrumentsURL(), 
                                      &payload, 
                                      response))
  {
    if (3 < response.GetPayload().size())
    {
      int startIndex = response.Contains("id", HTTPPayload::KeyVal::SUBKEY);
      while (-1 != startIndex)
      {
        std::string name = response.GetPayload()[startIndex + 8].val;
        std::string symbol = response.GetPayload()[startIndex + 11].val;
    
        Asset::Listing listing;
        listing.name = name;
        listing.symbol = symbol;
        listing.isCrypto = false;
        results.emplace_back(listing);
      
        startIndex = response.Contains("id", 
                                       HTTPPayload::KeyVal::SUBKEY, 
                                       startIndex + 1);
      }
    }
  }

  return response.GetPayload().size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchCryptoByName
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int RobinAPI::SearchCryptoBySymbol(std::string symbol,
                                            std::vector<Asset::Listing>& results)
{
  return Crypto::SearchBySymbol(symbol, results);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchCryptoByName
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int RobinAPI::SearchCryptoByName(std::string name,
                                          std::vector<Asset::Listing>& results)
{
  return Crypto::SearchByName(name, results);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildMarketOrders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::BuildMarketOrders()
{
  MarketOrders.clear();

  if (RobinConstants::STATUS_OK != Stock::RetrieveOrders())
  {
    return false;
  }

  if (RobinConstants::STATUS_OK != Crypto::RetrieveOrders())
  {
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddMarketOrder
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::AddMarketOrder(MarketOrder& order)
{
  MarketOrders.emplace_back(order);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetMarketOrders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<RobinAPI::MarketOrder>* RobinAPI::GetMarketOrders()
{
  return &MarketOrders;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  CancelStockOrder
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::CancelStockOrder(std::string orderId)
{
  if (true == IsLoggedIn())
  {
    if (0 == orderId.length())
    {
      return false;
    }

    // Post a request
    HTTPResponse response;
    HTTPSessionMgr::Instance()->POST(RobinConstants::CancelStockOrderURL(orderId), 
                                     EmptyRequest, 
                                     response);
    if (200 == response.Status)
    {
      return true;
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  CancelCryptoOrder
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::CancelCryptoOrder(std::string orderId)
{
  if (true == IsLoggedIn())
  {
    if (0 == orderId.length())
    {
      return false;
    }

    // Post a request
    HTTPResponse response;
    HTTPSessionMgr::Instance()->POST(RobinConstants::CancelCryptoOrderURL(orderId),
                                     EmptyRequest,
                                     response);
    if (200 == response.Status)
    {
      return true;
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SendStockOrder
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::SendStockOrder(std::string symbol, 
                              uint32_t quantity, 
                              bool purchase, 
                              float price)
{
  // Do not continue if we aren't logged in.
  if (false == IsLoggedIn())
  {
    return false;
  }

  // Retrieve instrument url
  std::string instrumentURL = "";
  std::vector<Asset*> assets = GetAssets();
  for (size_t i = 0; i < assets.size(); ++i)
  {
    if (0 == assets[i]->GetSymbol().compare(symbol))
    {
      instrumentURL = assets[i]->GetURL();
      break;
    }
  }

  // Do not continue if we couldn't find the symbol.
  if (0 == instrumentURL.length())
  {
    return false; // Instrument not found
  }

  // Build payload, for some reason urlencoded isn't supported, but json is.
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);
  payload.RemoveFromHeader("Content-Type");
  payload.AddToHeader("Content-Type", "application/json");

  payload.Add("account", "https://" + RobinConstants::AccountURL(ThisPortfolio->GetAccountNumber()));
  payload.Add("instrument", instrumentURL);
  payload.Add("symbol", symbol);

  std::stringstream stream;
  stream << std::fixed << std::setprecision(2);
  stream << price;
  payload.Add("price", stream.str());
  payload.Add("quantity", std::to_string(quantity));

  std::string ref_id = "";
  char temp_buf[64] = {0};
  sprintf_s(temp_buf, "%08x-%04x-%04x-%04x-%08x%04x",
            GenerateRandomKey(4),
            GenerateRandomKey(2),
            GenerateRandomKey(2),
            GenerateRandomKey(2),
            GenerateRandomKey(4),
            GenerateRandomKey(2));
  ref_id = temp_buf;

  payload.Add("ref_id", ref_id);
  payload.Add("type", "market");
  payload.Add("time_in_force", "gtc"); // Good til end of day
  payload.Add("trigger", "immediate");
  payload.Add("side", (true == purchase ? "buy" : "sell"));
  payload.Add("extended_hours", "False");

  // Post a request
  HTTPResponse response;
  HTTPSessionMgr::Instance()->POST(RobinConstants::StockOrdersURL(""), 
                                   &payload, 
                                   response);

  if (201 == response.Status)
  {
    // Extract order details and add it to pending orders
    
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SendCryptoOrder
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool RobinAPI::SendCryptoOrder(std::string symbol, 
                               uint32_t quantity,
                               bool purchase, 
                               float price)
{
  // Do not continue if we aren't logged in.
  if (false == IsLoggedIn())
  {
    return false;
  }

  // Retrieve instrument url
  std::string id = "";
  std::vector<Asset*> assets = GetAssets();
  for (size_t i = 0; i < assets.size(); ++i)
  {
    if (0 == assets[i]->GetSymbol().compare(symbol))
    {
      id = assets[i]->GetId();
      break;
    }
  }

  // Do not continue if we couldn't find the symbol.
  if (0 == id.length())
  {
    return false; // Instrument not found
  }

  // Build payload, for some reason urlencoded isn't supported, but json is.
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);
  payload.RemoveFromHeader("Content-Type");
  payload.AddToHeader("Content-Type", "application/json");

  payload.Add("account_id", ThisPortfolio->GetCryptoAccountId());
  payload.Add("currency_pair_id", id);

  std::stringstream stream;
  stream << std::fixed << std::setprecision(price < 1.0f ? 4 : 2);
  stream << price;
  payload.Add("price", stream.str());
  payload.Add("quantity", std::to_string(quantity));

  std::string ref_id = "";
  char temp_buf[64] = { 0 };
  sprintf_s(temp_buf, "%08x-%04x-%04x-%04x-%08x%04x",
                      GenerateRandomKey(4),
                      GenerateRandomKey(2),
                      GenerateRandomKey(2),
                      GenerateRandomKey(2),
                      GenerateRandomKey(4),
                      GenerateRandomKey(2));
  ref_id = temp_buf;

  payload.Add("ref_id", ref_id);
  payload.Add("side", (true == purchase ? "buy" : "sell"));
  payload.Add("time_in_force", "gtc"); // Good til canceled
  payload.Add("type", "market");

  // Post a request
  HTTPResponse response;
  HTTPSessionMgr::Instance()->POST(RobinConstants::CryptoOrdersURL(""), 
                                   &payload, 
                                   response);
  if (201 == response.Status)
  {
    return true;
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  FillRequestHeaders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void RobinAPI::FillRequestHeaders(HTTPRequest& request)
{
  std::vector<HTTPPayload::KeyVal>* header = &(request.GetHeader());
  header->clear();

  // Initialize headers
  /* Headers.Add("Accept-Encoding", "gzip,deflate,br"); // Not-Supported... */
  std::vector<HTTPPayload::KeyVal>* cHeader = &(EmptyRequest->GetHeader());
  for (unsigned int i = 0; i < cHeader->size(); ++i)
  {
    header->emplace_back(cHeader->at(i));
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GenerateRandomKey
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int RobinAPI::GenerateRandomKey(uint32_t bCnt)
{
  std::random_device device;
  std::mt19937::result_type seed =
    device() ^ static_cast<std::mt19937::result_type>
    (std::chrono::duration_cast<std::chrono::seconds>
      (std::chrono::system_clock::now().time_since_epoch()).count());
  std::mt19937 randGenerator(seed);
  std::uniform_int_distribution<int> distribution(1, 0xFF);

  // Generate random reference id
  unsigned int key = 0;
  for (size_t i = 0; i < bCnt; ++i)
  {
    key |= distribution(randGenerator) << (i * 8);
  }

  return key;
}