#pragma once
#include <string>

class RobinConstants
{
public:
  // URL String accessors
  static std::string LoginURL();
  static std::string PositionsURL();
  static std::string QuotesURL();
  static std::string QuotesHistoricalURL();
  static std::string AccountsURL();
  static std::string AccountURL(std::string accountNumber);
  static std::string PortfolioURL();
  static std::string PortfolioHistoricalURL(std::string accountNumber);
  static std::string InstrumentsURL();
  static std::string CryptoAccountURL();
  static std::string CryptoQuoteURL(std::string id);
  static std::string CryptoHistoricalURL(std::string id);
  static std::string UnifiedAccountURL();
  static std::string CryptoPairsURL();
  static std::string CryptoHoldingsURL();
  static std::string StockOrdersURL(std::string orderId);
  static std::string CryptoOrdersURL(std::string orderId);
  static std::string CancelStockOrderURL(std::string orderId);
  static std::string CancelCryptoOrderURL(std::string orderId);

  // Enumeration of status types
  enum StatusType
  {
    // Common statuses
    STATUS_OK,
    STATUS_BAD_CONNECTION,
    STATUS_LOGGED_OUT,
    STATUS_INVALID_PARAMS,

    // Authentication specific
    STATUS_BAD_CREDENTIALS,
    STATUS_2FA,
    STATUS_BAD_2FA,
    
    // Positions specific
    STATUS_NO_STOCKS,

    // Portfolio specific
    STATUS_NO_ACCOUNT,

    UNKNOWN_STATE
  };

  // Enumeration of historical types
  enum Interval
  {
    MINUTES_5,  
    MINUTES_10, 
    HALF_HOUR,  
    HOURLY,     
    DAILY,     
    WEEKLY,     
    MONTHLY,    
    DEFAULT_INTERVAL
  };

  enum Span
  {
    DAY,
    WEEK,
    MONTH,
    QUARTER,
    YEAR,
    HALF_DECADE
  };

  enum Bounds
  {
    EXTENDED,
    REGULAR,
    TRADING
  };

  static const char* IntervalParams[MONTHLY + 1];
  static const char* SpanParams[HALF_DECADE + 1];
  static const char* BoundsParams[TRADING + 1];
};

