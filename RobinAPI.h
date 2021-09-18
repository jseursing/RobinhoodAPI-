#pragma once
#include "Asset.h"
#include "HTTPPayload.h"
#include "Portfolio.h"
#include "RobinConstants.h"

// External declarations of support classes.

// API Class definition
class RobinAPI
{
// Friend class declarations
friend class Authentication;
friend class Stocks;
friend class Crypto;
friend class CryptoDatabase;
friend class Portfolio;

public:
  // Generic HTTP Request with headers pre-filled for RobinAPI
  static HTTPRequest* EmptyRequest;
  static HTTPRequest* GenericRequest;

  // Notification system
  struct PriceAlertEntry
  {
    float min;
    float max;
    bool minReady;
    bool maxReady;
    std::string symbol;
  };

  // Market Orders
  struct MarketOrder
  {
    bool crypto;
    std::string id;
    std::string symbol;
    std::string name;
    std::string status;  
  
    enum Type
    {
      BUY,
      SELL
    };
    Type type;

    float price;
    float quantity;
    std::string reject_reason;
    std::string last_date;
  };

  // Shorted stocks
  struct ShortedStock
  {
    std::string available;
    std::string fee;
    std::string timestamp;
  };

  // Accessible by any class, debug usage.
  enum DebugType
  {
    NO_LOG,
    LOG_ERRORS  = 1,
    LOG_TRAFFIC = 2,
    LOG_ALL     = 3
  };
  static DebugType DEBUG_LEVEL;
  static void Log(std::string data);

  // Constants
  static const unsigned int DEVICE_TOKEN_LEN = 36;

  // Account authentication functions
  static bool Initialize();
  static bool IsLoggedIn();
  static RobinConstants::StatusType InitiateLogin(const char* email,
                                                  const char* passw, 
                                                  const char* mfa_code);
  static void TestAuthorization();
  static void InitiateLogOut(bool removeSession = true);
  static void SetAuthorization(const char* deviceToken, 
                               const char* tokenType,
                               const char* accessToken,
                               const char* refreshToken);

  // Portfolio functions
  static bool BuildPortfolioProfile();
  static bool BuildAccountProfile();
  static bool BuildAssetsProfile();
  static bool BuildHistoricalProfile
    (RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL);
  static std::vector<Portfolio::HistoricalEntry>* GetPortfolioHistory();
  static std::string GetEquity();
  static std::string GetPrevClose();
  static std::string GetBuyingPower();
  static std::string GetCurrencyCode();

  // Stock functions
  static size_t GetStockShortData(const char* symbol, 
                                  std::vector<ShortedStock>& data);
  static bool AddStock(std::string symbol);
  static void AddOwnedStock(std::string url,
                            float quantity,
                            float averagePrice);

  // Crypto functions
  static bool AddCryptoBySymbol(std::string symbol);
  static bool AddOwnedCrypto(std::string symbol,
                             float quantity,
                             float averagePrice);

  // Asset functions
  static std::vector<Asset*>& GetAssets();
  static void AddPendingShare(Asset::Listing& asset);
  static bool RemoveAsset(std::string symbol);
  static bool BuildAssetsHistoricalData
    (unsigned int index, 
     RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL);
  static std::vector<Asset::HistoricalEntry>* GetAssetHistory(unsigned int index);
  static float GetLatestAssetPrice(std::string symbol, bool crypto);

  // Alert functions
  static PriceAlertEntry* GetPriceAlert(std::string symbol);
  static void SetPriceAlert(std::string symbol, float high, float low);
  static void SetAlertCallback(void* fn);

  // Algorithmic functions
  static void SetVolatilityRatio(float ratio);
  static float GetVolatilityRatio();
  static void SetVolatileLimit(unsigned int count);
  static unsigned int GetVolatilityLimit();

  // Search functions
  static unsigned int SearchStocks(std::string query, 
                                   std::vector<Asset::Listing>& results);
  static unsigned int SearchCryptoBySymbol(std::string symbol, 
                                           std::vector<Asset::Listing>& results);
  static unsigned int SearchCryptoByName(std::string name, 
                                         std::vector<Asset::Listing>& results);
  static bool CheckStockForVolatility(std::string symbol,
                                      RobinConstants::Span span);
  static bool CheckCryptoForVolatility(std::string symbol,
                                       RobinConstants::Span span);

  // Market functions
  static bool BuildMarketOrders();
  static void AddMarketOrder(MarketOrder& order);
  static std::vector<MarketOrder>* GetMarketOrders();
  static bool CancelStockOrder(std::string orderId);
  static bool CancelCryptoOrder(std::string orderId);
  static bool SendStockOrder(std::string symbol, uint32_t quantity, bool purchase, float price);
  static bool SendCryptoOrder(std::string symbol, uint32_t quantity, bool purchase, float price);

  // Other functions
  static void FillRequestHeaders(HTTPRequest& request);
  static void SetEmail(const char* value);
  static std::string& GetEmail();
  static void SetPassword(const char* value);
  static std::string& GetPassword();
  static void SetDeviceToken(const char* value);
  static std::string& GetDeviceToken();
  static void SetTokenType(const char* value);
  static std::string& GetTokenType();
  static void SetAccessToken(const char* value);
  static std::string& GetAccessToken();
  static void SetRefreshToken(const char* value);
  static std::string& GetRefreshToken();
  static std::string GetAuthorization();

  static unsigned int GenerateRandomKey(uint32_t bCnt);
  
  // Members 
  static bool LoggedIn;
  static std::string Path;
  static std::string Email;
  static std::string Passw;
  static std::string DeviceToken;
  static std::string TokenType;
  static std::string AccessToken;
  static std::string RefreshToken;
  static Portfolio* ThisPortfolio;
  static void* AlertCallback;
  static std::vector<Asset::Listing> PendingShares;
  static std::vector<PriceAlertEntry> PriceAlerts;
  static std::vector<MarketOrder> MarketOrders;

  static float VolatileDelta;
  static unsigned int VolatileLimit;
};

