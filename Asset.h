#pragma once
#include "RobinConstants.h"
#include <vector>

// Class definition
class Asset
{
public:
  // Generic ticker information
  struct Listing
  {
    std::string symbol;
    std::string name;
    std::string id;

    // Only used by search/import/export functions
    bool isCrypto;
  };

  // Historical data
  struct HistoricalEntry
  {
    std::string date;
    float open;
    float close;
    float hi;
    float lo;
    unsigned int volume;
  
    enum Session
    {
      PRE_MARKET,
      REGULAR,
      POST_MARKET
    };
    Session session;
  };

  // Functions
  std::string GetId() const;
  std::string GetName() const;
  std::string GetSymbol() const;
  std::string GetURL() const;
  std::string GetBidPrice() const;
  std::string GetAskPrice() const;
  std::string GetBidSize() const;
  std::string GetAskSize() const;
  std::string GetLastTradePrice() const;
  std::string GetPreviousClose() const;
  float GetQuantity() const;
  float GetAveragePrice() const;
  void SetSimulationDelta(float delta);
  std::vector<HistoricalEntry>* GetHistory();
  virtual RobinConstants::StatusType BuildHistoricalData
    (RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL) = 0;
  virtual RobinConstants::StatusType Refresh();
  void Update(float quantity, float price);
  Asset();
  ~Asset();

protected:
  std::string Id;
  std::string Name;
  std::string Symbol;
  std::string URL;
  std::string BidPrice;
  std::string AskPrice;
  std::string BidSize;
  std::string AskSize;
  std::string LastTradePrice;
  std::string PreviousClose;
  float Quantity;
  float AverageBuyPrice;
  float SimulationDelta;
  std::vector<HistoricalEntry> History;
};