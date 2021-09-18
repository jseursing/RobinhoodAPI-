#include "Crypto.h"
#include "HTTPSessionMgr.h"
#include "RobinAPI.h"
#include <algorithm>


// Static definition
std::vector<Asset::Listing> Crypto::Database;

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  LoadDatabase
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
bool Crypto::LoadDatabase()
{
  // Remove existing entries
  Database.clear();

  // Retrieve all crypto pairs
  HTTPResponse results;
  if (true == HTTPSessionMgr::Instance()->GET(RobinConstants::CryptoPairsURL(),
                                              RobinAPI::EmptyRequest,
                                              results))
  {
    std::vector<HTTPPayload::KeyVal> payload = results.GetPayload();

    int index = results.Contains("code", HTTPPayload::KeyVal::SUB0_SUBKEY);
    while (-1 != index)
    {
      Listing listing;
      listing.symbol = payload[index].val;;
      listing.name = payload[index + 3].val;
      listing.id = payload[index + 6].val;
      listing.isCrypto = true;
      std::transform(listing.name.begin(),
                     listing.name.end(),
                     listing.name.begin(),
                     std::toupper);
      Database.emplace_back(listing);

      index += 3;
      index = results.Contains("code", HTTPPayload::KeyVal::SUB0_SUBKEY, index);
    }
  }

  return (0 < Database.size());
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchBySymbol
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Crypto::SearchBySymbol(std::string symbol, std::vector<Listing>& results)
{
  if (0 < symbol.length())
  {
    for (unsigned int i = 0; i < Database.size(); ++i)
    {
      // Do not continue if the first symbol's character does not match.
      if (symbol[0] != Database[i].symbol[0])
      {
        continue;
      }

      // If the symbol contains our query, add it to results
      if (0 == Database[i].symbol.find(symbol.c_str()))
      {
        results.emplace_back(Database[i]);
      }
    }
  }

  return results.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchByName
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Crypto::SearchByName(std::string name, std::vector<Listing>& results)
{
  if (0 < name.length())
  {
    for (unsigned int i = 0; i < Database.size(); ++i)
    {
      // If the name contains our query, add it to results
      if (0 == Database[i].name.find(name.c_str()))
      {
        results.emplace_back(Database[i]);
      }
    }
  }

  return results.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  SearchByName
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int Crypto::SearchById(std::string id, std::vector<Listing>& results)
{
  if (0 < id.length())
  {
    for (unsigned int i = 0; i < Database.size(); ++i)
    {
      // If the name contains our query, add it to results
      if (0 == Database[i].id.find(id.c_str()))
      {
        results.emplace_back(Database[i]);
      }
    }
  }

  return results.size();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildPositions
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Crypto::BuildPositions()
{
  enum
  {
    COST_IDX = 2,
    QUANTITY_IDX = 3,
    CODE_IDX = 12,
    ENTrY_MAX = 16
  };

  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::CryptoHoldingsURL(), 
                                               RobinAPI::EmptyRequest, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Retrieve positions
  std::vector<HTTPPayload::KeyVal> data = response.GetPayload();
  int costIndex = response.Contains("cost_bases", HTTPPayload::KeyVal::SUBKEY);
  while (-1 != costIndex)
  {
    // Check for valid crypto
    if (HTTPPayload::KeyVal::SUB0_SUBKEY == data[costIndex + 1].tier)
    {
      // Retrieve data from current payload
      float quantity = stof(data[costIndex + QUANTITY_IDX].val);
      float directCost = stof(data[costIndex + COST_IDX].val);
      float averageBuyPrice = directCost / quantity;

      // Add/Update crypto
      RobinAPI::AddOwnedCrypto(data[costIndex + CODE_IDX].val, 
                               quantity, 
                               averageBuyPrice);
    }

    costIndex = response.Contains("cost_bases", HTTPPayload::KeyVal::SUBKEY, costIndex + 1);
  }

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RetrieveOrders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Crypto::RetrieveOrders()
{
  enum
  {
    ID_IDX = 0,
    PRICE_IDX = 2,
    QUANTITY_IDX = 3,
    TYPE_IDX = 6,
    STATE_IDX = 7,
    DATE_IDX = 10,
    PAIR_ID_IDX = 16
  };

  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::CryptoOrdersURL(""),
                                               RobinAPI::EmptyRequest,
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Begin parsing response and adding each market order.
  HTTPPayload::KeyVal::Tier tier = HTTPPayload::KeyVal::SUBKEY;
  int start = response.Contains("id", tier);
  if (-1 == start)
  {
    tier = HTTPPayload::KeyVal::SUB0_SUBKEY;
    start = response.Contains("id", HTTPPayload::KeyVal::SUB0_SUBKEY);
    if (-1 == start)
    {
      return RobinConstants::UNKNOWN_STATE;
    }
  }

  std::vector<HTTPPayload::KeyVal>& data = response.GetPayload();
  for (size_t i = start; i != -1; i = response.Contains("id", tier, i + 1))
  {
    // Check for end, exit if incomplete
    if ((i + PAIR_ID_IDX) >= data.size())
    {
      break;
    }

    RobinAPI::MarketOrder order;
    order.crypto = true;
    order.id = data[i + ID_IDX].val;
    order.price = stof(data[i + PRICE_IDX].val);
    order.quantity = stof(data[i + QUANTITY_IDX].val);
    order.type = (0 == data[i + TYPE_IDX].val.compare("buy") ? order.BUY : order.SELL);
    order.status = data[i + STATE_IDX].val;
    order.last_date = data[i + DATE_IDX].val;
    order.reject_reason = "";

    // Using the instrument URL, retrieve the symbol and name
    std::string id = data[i + PAIR_ID_IDX].val;

    std::vector<Asset::Listing> results;
    if (0 == SearchById(id, results))
    {
      continue;
    }

    // Retrieve data
    order.symbol = results[0].symbol;
    order.name = results[0].name;
    RobinAPI::AddMarketOrder(order);

    start = response.Contains("id", tier, start + 1);
  }

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildHistoricalData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Crypto::BuildHistoricalData(RobinConstants::Span span,
                                                       RobinConstants::Interval interval)
{
  enum
  {
    DATE_IDX,
    OPEN_IDX,
    CLOSE_IDX,
    HIGH_IDX,
    LOW_IDX,
    VOL_IDX,
    ENTRY_LEN = 8
  };

  // Search the database for matching symbol
  std::vector<Listing> results;
  if (0 == SearchBySymbol(Symbol, results))
  {
    return RobinConstants::STATUS_INVALID_PARAMS;
  }

  // Build payload using symbol and specified parameters.
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);

  switch (span)
  {
  case RobinConstants::DAY:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::MINUTES_5]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::DAY]);
    }
    break;
  case RobinConstants::WEEK:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::HALF_HOUR]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::WEEK]);
    }
    break;
  case RobinConstants::MONTH:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::DAILY]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::MONTH]);
    }
    break;
  case RobinConstants::QUARTER:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::DAILY]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::QUARTER]);
    }
    break;
  case RobinConstants::YEAR:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::WEEKLY]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::YEAR]);
    }
    break;
  case RobinConstants::HALF_DECADE:
    payload.Add("interval", RobinConstants::IntervalParams[RobinConstants::MONTHLY]);
    if (RobinConstants::DEFAULT_INTERVAL == interval)
    {
      payload.Add("span", RobinConstants::SpanParams[RobinConstants::HALF_DECADE]);
    }
    break;
  }

  if (RobinConstants::DEFAULT_INTERVAL != interval)
  {
    payload.Add("span", RobinConstants::SpanParams[interval]);
  }

  payload.Add("bounds", "24_7");

  // Send GET request 
  HTTPResponse response;
  std::string object = RobinConstants::CryptoHistoricalURL(results[0].id);
  if (false == HTTPSessionMgr::Instance()->GET(object, &payload, response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Find the first historical entry.
  int startIndex = response.Contains("begins_at", HTTPPayload::KeyVal::SUBKEY);
  if (-1 == startIndex)
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  // Now we interate through all of the payload entries and fill our historical data.
  History.clear();

  std::vector<HTTPPayload::KeyVal> data = response.GetPayload();
  for (int i = startIndex; i < (data.size() - 10); i += ENTRY_LEN)
  {
    History.push_back({ data[i + DATE_IDX].val,
                        stof(data[i + OPEN_IDX].val),              
                        stof(data[i + CLOSE_IDX].val),            
                        stof(data[i + HIGH_IDX].val),         
                        stof(data[i + LOW_IDX].val),             
                        strtoul(data[i + VOL_IDX].val.c_str(), 0, 10),
                        Asset::HistoricalEntry::REGULAR }); 
  }

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  GetLatestPrice
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Crypto::Refresh()
{
  // Search the database for matching symbol
  std::vector<Listing> results;
  if (0 == SearchBySymbol(Symbol, results))
  {
    return RobinConstants::STATUS_INVALID_PARAMS;
  }

  // Build GET parameters
  HTTPResponse response;
  std::string object = RobinConstants::CryptoQuoteURL(results[0].id);
  if (false == HTTPSessionMgr::Instance()->GET(object, RobinAPI::EmptyRequest, response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Retrieve the price
  LastTradePrice = response.Get("mark_price");
  PreviousClose = response.Get("open_price");
  BidPrice = response.Get("bid_price");
  BidSize = "-";
  AskPrice = response.Get("ask_price");
  AskSize = "-";

  return Asset::Refresh();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Crypto::Crypto(std::string id,
               std::string name,
               std::string symbol,
               float quantity,
               float averagePrice) :
  Asset()
{
  Id = id;
  Name = name;
  Symbol = symbol;
  Quantity = quantity;
  AverageBuyPrice = averagePrice;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Destructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Crypto::~Crypto()
{
  
}