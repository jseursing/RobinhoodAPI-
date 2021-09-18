#include "HTTPSessionMgr.h"
#include "RobinAPI.h"
#include "Stock.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildPositions
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Stock::BuildPositions()
{
  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::PositionsURL(),
                                               RobinAPI::EmptyRequest, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }
 
  // Perform a POST format to verify we are logged in, otherwise log out...
  if (-1 == response.Contains("results", HTTPPayload::KeyVal::KEY))
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  // Retrieve positions
  std::vector<HTTPPayload::KeyVal> payload = response.GetPayload();
  int startPos = response.Contains("instrument", HTTPPayload::KeyVal::SUBKEY);
  while (-1 != startPos)
  {
    // Retrieve data from current payload
    float averagePrice = stof(payload[startPos + 3].val);
    float quantity = stof(payload[startPos + 5].val);
    
    // Do not add 0 quanity stocks..
    if (0.00f < quantity)
    {
      // Perform GET request on instrument URL and retrieve rest of stock fields.
      std::string instrumentURL = payload[startPos].val;
      RobinAPI::AddOwnedStock(instrumentURL, quantity, averagePrice);
    }

    // Get next stock
    startPos = response.Contains("instrument", 
                                 HTTPPayload::KeyVal::SUBKEY, 
                                 startPos + 1);
  }

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  RetrieveOrders
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Stock::RetrieveOrders()
{
  enum
  {
    ID_IDX = 0,
    INSTRUMENT_IDX = 6
  };

  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::StockOrdersURL(""),
                                               RobinAPI::EmptyRequest,
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Begin parsing response and adding each market order.
  HTTPPayload::KeyVal::Tier tier = HTTPPayload::KeyVal::SUBKEY;
  int start = response.Contains("id", HTTPPayload::KeyVal::SUBKEY);
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
    RobinAPI::MarketOrder order;
    order.crypto = false;
    order.id = data[i + ID_IDX].val;

    // Using the instrument URL, retrieve the symbol and name
    HTTPResponse instrumentResp;
    std::string instrument = data[i + INSTRUMENT_IDX].val;
    if (false == HTTPSessionMgr::Instance()->GET(instrument, 
                                                 RobinAPI::EmptyRequest, 
                                                 instrumentResp))
    {
      continue;
    }

    // Retrieve data
    order.symbol = instrumentResp.Get("symbol");
    order.name = instrumentResp.Get("simple_name");
    
    int fieldIdx = response.Contains("state", HTTPPayload::KeyVal::SUBKEY, i);
    if (-1 != fieldIdx)
    {
      order.status = data[fieldIdx].val;
    }

    fieldIdx = response.Contains("side", HTTPPayload::KeyVal::SUBKEY, i);
    if (-1 != fieldIdx)
    {
      order.type = (0 == data[fieldIdx].val.compare("buy") ? order.BUY : order.SELL);
    }

    // Verify we don't try to convert a null price
    std::string price = "null";
    fieldIdx = response.Contains("price", HTTPPayload::KeyVal::SUBKEY, i);
    if (-1 != fieldIdx)
    {
      price = data[fieldIdx].val;
    }

    if (0 == price.compare("null"))
    {
      fieldIdx = response.Contains("average_price", HTTPPayload::KeyVal::SUBKEY, i);
      if (-1 != fieldIdx)
      {
        price = data[fieldIdx].val;
      }
    }

    order.price = (0 == price.compare("null") ? 0.0f : stof(price));

    fieldIdx = response.Contains("quantity", HTTPPayload::KeyVal::SUBKEY, i);
    if (-1 != fieldIdx)
    {
      order.quantity = stof(data[fieldIdx].val);
    }

    fieldIdx = response.Contains("updated_at", HTTPPayload::KeyVal::SUBKEY, i);
    if (-1 != fieldIdx)
    {
      order.last_date = data[fieldIdx].val;
    }

    RobinAPI::AddMarketOrder(order);
  }

  return RobinConstants::STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  BuildHistoricalData
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Stock::BuildHistoricalData(RobinConstants::Span span,
                                                      RobinConstants::Interval interval)
{
  // Enum containing indices for fields
  enum
  {
    DATE_IDX,
    OPEN_IDX,
    CLOSE_IDX,
    HIGH_IDX,
    LOW_IDX,
    VOL_IDX,
    SESSION_IDX,
    ENTRY_LEN = 8
  };

  // Validate passed in parameters 
  if (RobinConstants::HALF_DECADE < span)
  {
    return RobinConstants::STATUS_INVALID_PARAMS;
  }

  // Build payload using symbol and specified parameters.
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);
  payload.Add("symbols", Symbol);
  
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

  // Send GET request with dummy payload
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::QuotesHistoricalURL(), 
                                               &payload, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Find the first historical entry.
  int startIndex = response.Contains("begins_at", HTTPPayload::KeyVal::SUB0_SUBKEY);
  if (-1 != startIndex)
  {
    // Now we interate through all of the payload entries and fill our historical data.
    History.clear();

    std::vector<HTTPPayload::KeyVal> data = response.GetPayload();
    for (int i = startIndex; i < (data.size() - 2); i += ENTRY_LEN)
    {
      Asset::HistoricalEntry::Session type;
      switch (data[i + SESSION_IDX].val[2])
      {
        case 'e': // pre
          type = Asset::HistoricalEntry::PRE_MARKET;
          break;
        case 'g': // reg
          type = Asset::HistoricalEntry::REGULAR;
          break;
        case 's': // post
          type = Asset::HistoricalEntry::POST_MARKET;
          break;
      }

      History.push_back({ data[i + DATE_IDX].val, 
                          stof(data[i + OPEN_IDX].val),               
                          stof(data[i + CLOSE_IDX].val),              
                          stof(data[i + HIGH_IDX].val),              
                          stof(data[i + LOW_IDX].val),              
                          strtoul(data[i + VOL_IDX].val.c_str(), 0, 10),
                          type }); 
    }

    return RobinConstants::STATUS_OK;
  }
  else
  {
    startIndex = response.Contains("open_price", HTTPPayload::KeyVal::SUBKEY);
    if (-1 != startIndex)
    {
      std::string price = response.Get("results", "open_price");
      History.push_back({ "Now",
                          stof(price),
                          stof(price),
                          History.back().hi,
                          History.back().lo,
                          History.back().volume });

      return RobinConstants::STATUS_OK;
    }
  }

  return RobinConstants::STATUS_LOGGED_OUT;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Refresh
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
RobinConstants::StatusType Stock::Refresh()
{
  HTTPRequest payload;
  RobinAPI::FillRequestHeaders(payload);
  payload.Add("symbols", Symbol);

  // Send GET request
  HTTPResponse response;
  if (false == HTTPSessionMgr::Instance()->GET(RobinConstants::QuotesURL(), 
                                               &payload, 
                                               response))
  {
    return RobinConstants::STATUS_BAD_CONNECTION;
  }

  // Find the first historical entry.
  int startIndex = response.Contains("ask_price", HTTPPayload::KeyVal::SUBKEY);
  if (-1 == startIndex)
  {
    return RobinConstants::STATUS_LOGGED_OUT;
  }

  // Retrieve the last trade price and previous close
  LastTradePrice = response.Get("results", "last_extended_hours_trade_price");
  if (0 == LastTradePrice.compare("null"))
  {
    LastTradePrice = response.Get("results", "last_trade_price");
  }

  // Insert simulation data
  if (0.0f != SimulationDelta)
  {
    float simPrice = stof(LastTradePrice) + SimulationDelta;
    SimulationDelta += SimulationDelta;
    LastTradePrice = std::to_string(simPrice);
  }

  // Retrieve other data
  PreviousClose = response.Get("results", "previous_close");
  BidPrice = response.Get("results", "bid_price");
  BidSize = response.Get("results", "bid_size");
  AskPrice = response.Get("results", "ask_price");
  AskSize = response.Get("results", "ask_size");

  return Asset::Refresh();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Constructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Stock::Stock(std::string id,
             std::string name,
             std::string symbol,
             std::string url,
             float quantity,
             float avgBuyPrice) :
  Asset()
{
  Id = id;
  Name = name;
  Symbol = symbol;
  URL = url;
  Quantity = quantity;
  AverageBuyPrice = avgBuyPrice;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//  Function:  Destructor
//  Notes:     None
//
/////////////////////////////////////////////////////////////////////////////////////////
Stock::~Stock()
{
}