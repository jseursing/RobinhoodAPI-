#include "RobinConstants.h"



// Static definitions
const char* RobinConstants::IntervalParams[MONTHLY + 1] =
{
  "5minute\0",
  "10minute\0",
  "30minute\0",
  "hour\0",
  "day\0",
  "week\0",
  "month\0"
};

const char* RobinConstants::SpanParams[HALF_DECADE + 1] =
{
  "day\0",
  "week\0",
  "month\0",
  "3month\0",
  "year\0",
  "5year\0"
};

const char* RobinConstants::BoundsParams[TRADING + 1] =
{
  "extended\0",
  "regular\0",
  "trading\0"
};


/////////////////////////////////////////////////////////////////////////////////////////
//
// The following functions return a formatted/non-formatted string URL
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string RobinConstants::LoginURL()
{
  std::string url = "api.robinhood.com/oauth2/token/";

  return url;
}

std::string RobinConstants::PositionsURL()
{
  std::string url = "api.robinhood.com/positions/";

  return url;
}

std::string RobinConstants::QuotesURL()
{
  std::string url = "api.robinhood.com/quotes/";

  return url;
}

std::string RobinConstants::QuotesHistoricalURL()
{
  std::string url = "api.robinhood.com/quotes/historicals/";

  return url;
}

std::string RobinConstants::AccountsURL()
{
  std::string url = "api.robinhood.com/accounts/";

  return url;
}

std::string RobinConstants::AccountURL(std::string accountNumber)
{
  std::string url = "api.robinhood.com/accounts/" + accountNumber + "/";

  return url;
}

std::string RobinConstants::PortfolioURL()
{
  std::string url = "api.robinhood.com/portfolios/";

  return url;
}

std::string RobinConstants::PortfolioHistoricalURL(std::string accountNumber)
{
  std::string url = "api.robinhood.com/portfolios/historicals/" + accountNumber + "/\0";

  return url;
}

std::string RobinConstants::InstrumentsURL()
{
  std::string url = "api.robinhood.com/instruments/";

  return url;
}

std::string RobinConstants::CryptoAccountURL()
{
  std::string url = "nummus.robinhood.com/accounts/";

  return url;
}

std::string RobinConstants::CryptoQuoteURL(std::string id)
{
  std::string url = "api.robinhood.com/marketdata/forex/quotes/" + id + "/\0";

  return url;
}

std::string RobinConstants::CryptoHistoricalURL(std::string id)
{
  std::string url = "api.robinhood.com/marketdata/forex/historicals/" + id + "/\0";

  return url;
}

std::string RobinConstants::UnifiedAccountURL()
{
  std::string url = "phoenix.robinhood.com/accounts/unified";

  return url;
}

std::string RobinConstants::CryptoPairsURL()
{
  std::string url = "nummus.robinhood.com/currency_pairs/";

  return url;
}

std::string RobinConstants::CryptoHoldingsURL()
{
  std::string url = "nummus.robinhood.com/holdings/";

  return url;
}

std::string RobinConstants::StockOrdersURL(std::string orderId)
{
  std::string url = "api.robinhood.com/orders/";
  if (0 != orderId.length())
  {
    url += orderId + "/";
  }

  return url;
}

std::string RobinConstants::CryptoOrdersURL(std::string orderId)
{
  std::string url = "nummus.robinhood.com/orders/";
  if (0 != orderId.length())
  {
    url += orderId + "/";
  }

  return url;
}

std::string RobinConstants::CancelStockOrderURL(std::string orderId)
{
  std::string url = "api.robinhood.com/orders/" + orderId + "/cancel/";

  return url;
}

std::string RobinConstants::CancelCryptoOrderURL(std::string orderId)
{
  std::string url = "nummus.robinhood.com/orders/" + orderId + "/cancel/";

  return url;
}