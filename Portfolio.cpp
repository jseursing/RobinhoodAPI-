#include "HTTPSessionMgr.h"
#include "Portfolio.h"
#include "Crypto.h"
#include "Stock.h"
#include "RobinAPI.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildPortfolioProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Portfolio::BuildPortfolioProfile()
{
  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::UnifiedAccountURL(),
                                               RobinAPI::EmptyRequest, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  if (true == response.KeyContainsValue("X-Frame-Options:", "deny"))
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  // Extract data.
  if (-1 == response.Contains("account_buying_power"))
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  Equity = response.Get("total_equity", "amount");
  PrevClose = response.Get("portfolio_previous_close", "amount");
  BuyingPower = response.Get("account_buying_power", "amount");
  CurrencyCode = response.Get("portfolio_equity", "currency_code");

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  LoadAccountProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Portfolio::LoadAccountProfile()
{
  // Send GET request for portfolio profile
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::AccountsURL(), 
                                               RobinAPI::EmptyRequest, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Extract data.
  if (-1 == response.Contains("results"))
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  AccountNumber = response.Get("results", "account_number");
  UserId = response.Get("results", "user_id");

  // Set GET request for crypto profile
  response.Clear();
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::CryptoAccountURL(),
                                               RobinAPI::EmptyRequest,
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  CryptoAccountId = response.Get("results", "id");

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildHistoricalProfile
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType 
Portfolio::BuildHistoricalProfile(RobinConstants::Span span,
                                  RobinConstants::Interval interval)
{
  // Do not continue if account number isn't found
  if (0 == AccountNumber.length())
  {
    return RobinConstants::STATUS_NO_ACCOUNT;
  }

  // Validate passed in parameters 
  if (RobinConstants::HALF_DECADE < span)
  {
    return RobinConstants::STATUS_INVALID_PARAMS;
  }

  // Build payload using account number and specified parameters.
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);

  switch (span)
  {
  case RobinConstants::DAY:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::DAY]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::MINUTES_5]);
    }
    break;
  case RobinConstants::WEEK:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::WEEK]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::HALF_HOUR]);
    }
    break;
  case RobinConstants::MONTH:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::MONTH]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::DAILY]);
    }
    break;
  case RobinConstants::QUARTER:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::QUARTER]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::DAILY]);
    }
    break;
  case RobinConstants::YEAR:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::YEAR]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::WEEKLY]);
    }
    break;
  case RobinConstants::HALF_DECADE:
    payload.Add("span", RobinConstants::SpanParams[RobinConstants::HALF_DECADE]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::MONTHLY]);
    }
    break;
  }

  if (RobinConstants::DEFAULT_INTERVAL != interval)
  {
    payload.Add("interval", RobinConstants::IntervalParams[interval]);
  }

  payload.Add("bounds", (RobinConstants::DAY == span ?
                         RobinConstants::BoundsParams[RobinConstants::EXTENDED] :
                         RobinConstants::BoundsParams[RobinConstants::REGULAR]));

  // Send GET request
  HTTPResponse response;
  std::string object = RobinConstants::PortfolioHistoricalURL(AccountNumber);
  if (false == HTTPSessionMgr::Instance()->GET(object, &payload, response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Find the first historical entry.
  int startIndex = response.Contains("begins_at", HTTPPayload::KeyVal::SUBKEY);
  if (-1 != startIndex)
  {
    // Now we interate through all of the payload entries and fill our historical data.
    History.clear();
  
    std::vector<HTTPPayload::KeyVal> data = response.GetPayload();
    for (int i = startIndex; i < (data.size() - 1); i+=9)
    {
      History.push_back({ data[i].val, stof(data[i-4].val), stof(data[i-3].val) });
    }
  }

  // Add today's value
  History.push_back({ "Now", stof(Equity), stof(Equity) });

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetUserId
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetUserId() const
{
  return UserId;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetEquity
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetEquity() const
{
  return Equity;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetPrevClose
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetPrevClose() const
{
  return PrevClose;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetBuyingPower
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetBuyingPower() const
{
  return BuyingPower;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetCurrencyCode
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetCurrencyCode() const
{
  return CurrencyCode;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAccountNumber
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetAccountNumber() const
{
  return AccountNumber;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetCryptoAccountId
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::string Portfolio::GetCryptoAccountId() const
{
  return CryptoAccountId;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetHistory
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Portfolio::HistoricalEntry>* Portfolio::GetHistory()
{
  return &History;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAssetBySymbol
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
int Portfolio::GetAssetBySymbol(std::string symbol) const
{
  int index = -1;
  for (size_t i = 0; i < Assets.size(); ++i)
  {
    if (0 == Assets[i]->GetSymbol().compare(symbol))
    {
      index = i;
      break;
    }
  }

  return index;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddStock
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
void Portfolio::AddStock(std::string url, float quantity, float averagePrice)
{
  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(url, RobinAPI::EmptyRequest, response))
  {
    return;
  }

  // Retrieve data
  std::string id = response.Get("id");
  std::string name = response.Get("simple_name");
  std::string symbol = response.Get("symbol");

  // If the symbol exists, update the name and id, otherwise
  // add new stock.
  bool found = false;
  for (unsigned int i = 0; i < Assets.size(); ++i)
  {
    if (nullptr == dynamic_cast<Crypto*>(Assets[i]))
    {
      if (0 == Assets[i]->GetSymbol().compare(symbol.c_str()))
      {
        // If we found it, update quantity and averagePrice
        Assets[i]->Update(quantity, averagePrice);
        found = true;
        break;
      }
    }
  }

  if (false == found)
  {
    // Construct the new asset
    Assets.emplace_back(new Stock(id, name, symbol, url, quantity, averagePrice));
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddStock
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool Portfolio::AddStock(std::string symbol)
{
  // Check to see if the instrument URL exists, then add the stock.
  HTTPRequest payload;
  payload.Add("symbol", symbol);
  RobinAPI::FillRequestHeaders(payload);

  HTTPResponse response;
  if (false != HTTPSessionMgr::Instance()->GET(RobinConstants::InstrumentsURL(), 
                                               &payload, 
                                               response))
  {
    // Retrieve data
    std::string instrumentURL = response.Get("results", "url");
    if (0 != instrumentURL.length())
    {
      AddStock(instrumentURL, 0, 0);

      return true;
    }
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  AddCrypto
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool Portfolio::AddCrypto(std::string symbol, float quantity, float averagePrice)
{
  // Search the database for matching symbol
  std::vector<Crypto::Listing> results;
  if (0 == Crypto::SearchBySymbol(symbol, results))
  {
    return false;
  }

  // Retrieve data
  std::string id = results[0].id;
  std::string name = results[0].name;

  bool found = false;
  for (unsigned int i = 0; i < Assets.size(); ++i)
  {
    if (nullptr != dynamic_cast<Crypto*>(Assets[i]))
    {
      if (0 == Assets[i]->GetSymbol().compare(symbol.c_str()))
      {
        Assets[i]->Update(quantity, averagePrice);
        found = true;
        break;
      }
    }
  }

  if (false == found)
  {
    // Construct the new asset
    Assets.emplace_back(new Crypto(id, name, symbol, quantity, averagePrice));
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RemoveAsset
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool Portfolio::RemoveAsset(std::string symbol)
{
  bool success = false;

  for (unsigned int i = 0; i < Assets.size(); ++i)
  {
    if (0 == Assets[i]->GetSymbol().compare(symbol))
    {
      delete Assets[i];
      Assets.erase(Assets.begin() + i);
      success = true;
      break;
    }
  }

  return success;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetAssets
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<Asset*>& Portfolio::GetAssets()
{
  return Assets;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Portfolio::Portfolio() :
  Equity(""),
  PrevClose(""),
  BuyingPower(""),
  AccountNumber(""),
  UserId("")
{

}