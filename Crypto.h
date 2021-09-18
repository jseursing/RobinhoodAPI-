#pragma once
#include "Asset.h"


class Crypto : public Asset
{
public:
  static bool LoadDatabase();
  static unsigned int SearchBySymbol(std::string symbol, std::vector<Listing>& results);
  static unsigned int SearchByName(std::string name, std::vector<Listing>& results);
  static unsigned int SearchById(std::string id, std::vector<Listing>& results);

  // Crypto asset functions
  static RobinConstants::StatusType BuildPositions();
  static RobinConstants::StatusType RetrieveOrders();
  virtual RobinConstants::StatusType BuildHistoricalData
    (RobinConstants::Span span,
     RobinConstants::Interval interval = RobinConstants::DEFAULT_INTERVAL);
  virtual RobinConstants::StatusType Refresh();

  Crypto(std::string id,
         std::string name,
         std::string symbol,
         float quantity,
         float averagePrice);
  ~Crypto();

private:
  static std::vector<Listing> Database;
};

