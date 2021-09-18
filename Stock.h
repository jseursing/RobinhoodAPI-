#pragma once
#include "Asset.h"

class Stock : public Asset
{
public:
  static RobinConstants::StatusType BuildPositions();
  static RobinConstants::StatusType RetrieveOrders();
  virtual RobinConstants::StatusType BuildHistoricalData
    (RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL);
  virtual RobinConstants::StatusType Refresh();

  Stock(std::string id,
        std::string name,
        std::string symbol,
        std::string url,
        float quantity,
        float averagePrice);
  ~Stock();

private:
  
};

